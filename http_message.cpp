#include "http_message.h"
#include <sys/time.h>
#include "utils.h"
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include "zlib.h"

// #define DEBUG

#ifdef DEBUG
#include <errno.h>

#ifdef VIEWER_ONLY
extern void log(const char *, ...);
#else
#include <stdio.h>
#ifndef WIN32
#define log(x, args...) fprintf(stderr, x, ##args)
#else
#define log(x, args...) do {char buffer[4192]; snprintf(buffer, sizeof(buffer), x, ##args); OutputDebugString(buffer); } while(0)
#endif
#endif
#else
#define log(x, args...)
#endif


#ifdef WIN32

// su mingw (e su VC6) ci manca mstcpip.h

struct tcp_keepalive {
    u_long  onoff;
    u_long  keepalivetime;
    u_long  keepaliveinterval;
};
#define SIO_KEEPALIVE_VALS    _WSAIOW(IOC_VENDOR,4)

#endif

static char HeaderPart1[] = "HTTP/1.0 200 OK\r\nServer: notifyd/0.01\r\nDate: ";
static char HeaderPart2[] = "\r\nContent-type: ";
static char HeaderPart3[] = "\r\nLast-modified: ";
static char HeaderPart4[] = "\r\nContent-length: ";
static char HeaderPart5[] = "\r\nExpires: ";
static char HeaderPart6[] = "\r\n\r\n";
static char XML_Header[]  = "<?xml version='1.0' encoding='UTF-8' standalone='no'?>\r\n\r\n";

static char GetStringHeader[] = 
                                "User-Agent: Mozilla/5.0 (X11; Ubuntu; Linux i686; rv:32.0) Gecko/20100101 Firefox/32.0\r\n"
                                "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
                                "Accept-Language: it,en-us;q=0.7,en;q=0.3\r\n"
                                "Accept-Encoding: gzip, deflate\r\n"
                                "DNT: 1\r\n"
                                "Connection: keep-alive\r\n"
                                "Referer: http://img.playa-games.com/sfgame555.swf\r\n"
                                "\r\n";

int connect_time (int sock, struct sockaddr *addr, int size_addr, int timeout)
{
    int error = 0;
    fd_set	rset, wset;
    int 	n;
    struct timeval tval;

#ifndef WIN32
    int flags = fcntl (sock, F_GETFL, 0);
    fcntl (sock, F_SETFL, flags | O_NONBLOCK);		// set the socket as nonblocking IO
#else
    unsigned long par = 1;
    ioctlsocket(sock, FIONBIO, &par);
#endif

    if ((n = connect (sock, addr, size_addr)) < 0) {		// we connect, but it will return soon
        if (errno != EINPROGRESS && errno != EALREADY) {
            return -1;
        }
    }

    if (n == 0)
        goto done;

    FD_ZERO (&rset);
    FD_ZERO (&wset);
    FD_SET (sock, &rset);
    FD_SET (sock, &wset);
    tval.tv_sec = timeout / 1000;
    tval.tv_usec = (timeout%1000) * 1000;

    // We "select()" until connect() returns its result or timeout

    if ( (n = select(sock+1, &rset, &wset, NULL, timeout ? &tval : NULL)) == 0) {	
        close (sock);
        errno = ETIMEDOUT;
        return -1;
    }

    if (FD_ISSET(sock, &rset) || FD_ISSET(sock, &wset)) {
        socklen_t len = sizeof(error);
        if (getsockopt (sock, SOL_SOCKET, SO_ERROR, (char *) &error, &len) < 0) {
            errno = error;
            return -1;
        }
    } else {
        return -1;
    }
done:
#ifndef WIN32
    /* We change the socket options back to blocking IO */
    fcntl (sock, F_SETFL, flags);
#else
    par = 0;
    ioctlsocket(sock, FIONBIO, &par);
#endif
    return 0;
}


bool
HTTPMessage::Connect(const std::string &host, unsigned short port, int timeout_msec)
{
    struct sockaddr_in addr_out; 

    if ((sock_ = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return false;
    }

    memset(&addr_out, 0, sizeof(addr_out));
    addr_out.sin_family = AF_INET;
    addr_out.sin_port = htons(port);

    if ((addr_out.sin_addr.s_addr = inet_addr(host.c_str())) == INADDR_NONE) {
        sclose(sock_);
        return false;
    }

/* Il keepalive della connessione su sistemi unix
    int optval = 1;
    size_t optlen = sizeof(optval);
    ::setsockopt(sock_, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
 */

    // su win devo anche cambiare il valore di 7200000msec per il keepalive
#ifdef WIN32
    tcp_keepalive kp;
    DWORD returned;
    kp.onoff = 1;
    kp.keepalivetime = 5000; // ogni 5 secondi
    kp.keepaliveinterval = 1000; // un secondo
    WSAIoctl(sock_, SIO_KEEPALIVE_VALS, (LPVOID) &kp,
             sizeof(kp), NULL, 0, &returned, NULL, NULL);
#endif

    int rc;
    if (timeout_msec < 0)
        rc = connect(sock_, (struct sockaddr *)&addr_out, sizeof(addr_out));
    else
        rc = connect_time(sock_, (struct sockaddr *)&addr_out, sizeof(addr_out), timeout_msec);

    if (rc == -1) {
        sclose(sock_);
        return false;
    }

    offset_ = 0;

    return true;
}

bool
HTTPMessage::AskData(const std::string &path)
{
    std::ostringstream s;

    s << "GET ";

    if (path[0] != '/')
        s << '/';
    s << path << " HTTP/1.1\r\n"
      << "Host: " << servername_ << "\r\n"
      << GetStringHeader;

    return (send(sock_, s.str().c_str(), s.str().size(), 0) == (int)s.str().size());
}

std::string HTTPMessage::get_rfc1123_time(void)
{
	time_t tv;
	struct tm *tms;
    char buffer[64];
        
	tv = time(NULL);
#ifndef WIN32
    struct tm tmstatic;
	tms = localtime_r(&tv, &tmstatic); /* Make sure tz is set first time... */
#else
    tms = localtime(&tv); // su win32 localtime usa un buffer thread-specific
#endif
	strftime(buffer, 64, "%a, %d %b %Y %H:%M:%S GMT%z", tms);
	return buffer;
}

const char *XMLHeader()
{
    return XML_Header;
}

int HTTPMessage::
WaitMsg(int timeout, std::string &result)
{
    fd_set set;
    int rc2 = QUIT; // di default esco perche' la condizione non e' prevista!

    if (offset_) { // ci sono dati quindi non occorre aspettare
        rc2 = GetMsg(result);        
    }
    else {
        FD_ZERO(&set);
        FD_SET(sock_, &set);

        struct timeval to;

        to.tv_sec = timeout / 1000;
        to.tv_usec = (timeout % 1000) * 1000;

        int rc = select(sock_+1, &set, NULL, NULL, &to);

        if (rc < 0) {
            throw std::string("Attenzione errore in select");
        }
        else if (rc == 0)
            return TIMEOUT;
        else if (FD_ISSET(sock_, &set)) {
            rc2 = GetMsg(result);
        }
    }

    return rc2;
}

int HTTPMessage::
WaitRawData(int timeout, std::string &result)
{
    char buffer[8192];
    fd_set set;
    FD_ZERO(&set);
    FD_SET(sock_, &set);

    struct timeval to;

    to.tv_sec = timeout / 1000;
    to.tv_usec = (timeout % 1000) * 1000;

    int rc = select(sock_+1, &set, NULL, NULL, &to);

    if (rc < 0) {
        return ERR;
    }
    else if (FD_ISSET(sock_, &set)) {
        int len = ::recv(sock_, buffer, sizeof(buffer), 0);

        if (len == 0)
            return QUIT;
        else {
            result.assign(buffer, len);
            return OK;
        }
    }

    return TIMEOUT;
}

std::string HTTPMessage::
PrepareMsg(const std::string &body, const std::string &type)
{
    std::string now = get_rfc1123_time();

    std::string msg = HeaderPart1;
    msg += now;
    msg += HeaderPart2;
    msg += type;
    msg += HeaderPart3;
    msg += now;
    msg += HeaderPart4;
    msg += to_string(body.size());
    msg += HeaderPart5;
    msg += now;
    msg += HeaderPart6;
    msg += body;

    return msg;
}

bool HTTPMessage::
SendMsg(int socket, const std::string &body, const std::string &type)
{
        if (body.empty()) {
                return true;
        }

        std::string msg = PrepareMsg(body, type);        
        return (send(socket, &msg[0], msg.size(), 0) == (int) msg.size());
}

std::string HTTPMessage::
PrepareXML(const std::string &body)
{
    std::string msg = XML_Header;

    msg += body;

    return PrepareMsg(msg, "text/xml");
}

bool HTTPMessage::
SendXML(const std::string &body)
{
    std::string msg = PrepareXML(body);
    return (::send(sock_, &msg[0], msg.size(), 0) == (int) msg.size());
}

int HTTPMessage::
GetBytes(unsigned char *b, int len, int retries)
{
    int bytes = recv(sock_, (char *) b, len, 0); 

    if (bytes <= 0) {
        // gestisco socket non bloccante
        if (bytes == 0 || ERRNO != EWOULDBLOCK)
            return -1;
        else
            bytes = 0;
    }

    while (bytes < len && retries > 0) {
        retries--;
        MSleep(20);
        int r = recv(sock_,(char *)b + bytes, len - bytes, 0); 

        // gestisco socket non bloccante
        if (r < 0 && ERRNO == EWOULDBLOCK)
            continue;

        if (r <= 0)
            return -1;

        bytes += r;
    }

    return bytes;
}

bool HTTPMessage::
StripHeader()
{
    do  {
        int l = recv(sock_, buffer_ + offset_, 1, 0);

        if (l < 0 && ERRNO == EWOULDBLOCK) {
            MSleep(10);
            continue;
        }

        if (l <= 0) {
            return false;
        }
        offset_ += l;
        buffer_[offset_] = 0;
    }
    while (!HeaderAvailable());

    return true;
}

int HTTPMessage::
GetMsg(std::string &body)
{
    int rc;
   
    log("Entro in GetMsg con offset di %d\n", offset_);

    do {
        int l = 0;
       
        buffer_[offset_] = 0;

        while (!HeaderAvailable()) {
            l = recv(sock_, buffer_ + offset_, 4, 0);

            if (l < 0 && ERRNO == EWOULDBLOCK) {
                MSleep(10);
                continue;
            }

            if (l <= 0) {
                log("Recv ha ritornato: %d (offset: %d) (errno: %d, %s)\n",
                        l, offset_, errno, strerror(errno));
                return HTTPMessage::QUIT;
            }
            
            offset_ += l;
        }
        
        buffer_[offset_] = 0;

        log( "Ricevuti %d bytes, totali %d\n", l, offset_);

        log("buffer: %s\n", buffer_);

        rc = GetMsg(offset_, body);

        if (rc == HTTPMessage::MORE_DATA) {
            log( "Need more data: %d %d body:%d\n", l, offset_, body.size());
        }
    }
    while(rc == HTTPMessage::MORE_DATA);
    
    return rc;
}

bool HTTPMessage::
HeaderAvailable()
{
    char *start_header = strstr(buffer_, "HTTP/"), 
         *end_header = strstr(buffer_, "\r\n\r\n");
    
    if (!end_header)
        end_header = strstr(buffer_, "\n\n");

    if (!end_header)
        end_header = strstr(buffer_, "\n\r\n\r");

    if (!start_header || !end_header)
        return false;

    if (start_header > end_header)
        return false;

    //log("diff %d, offset: %d\n", end_header - buffer_, offset_);

    // voglio almeno 8 byte oltre la fine dell'header se il trasferimento e' di tipo chunked 
    if (strstr(buffer_, "ing: chunked") &&
        ( end_header - buffer_) > (offset_ - 10) ) 
        return false;

    return true;
}

int HTTPMessage::
GetMsg(int len, std::string &body)
{
    log("Avvio il parse del messaggio di %d bytes\n", len); 
    bool zipped = false;
    char *end_header;

    if (!strstr(buffer_, "HTTP/") ||
            !(end_header = strstr(buffer_, "\r\n\r\n")))
        return MORE_DATA; // header non completato.

    int header_len = end_header - buffer_ + 4; // il +4 e' la lunghezza del \r\n\r\n

    char *body_length = strstr(buffer_, "Content-length");

    if (!body_length)
        body_length = strstr(buffer_, "Content-Length");

    if (strstr(buffer_, "Content-Encoding: gzip") ||
        strstr(buffer_, "Content-Encoding: deflate"))
        zipped = true;

    bool ok_on_close = false;
    bool chunked = false;

    int body_len;

    if (!body_length) {
        if (strstr(buffer_, "Encoding: chunked")) {
            body_len = strtol(buffer_ + header_len, NULL, 16);
            char *pos = strchr(buffer_ + header_len, '\n');
            header_len = pos - buffer_ + 1;
            log("Spostato offset buffer, primo char: %d\n", *(buffer_ + header_len));
            chunked = true;
        }
        else {
            log("Procedo senza content length...\n");
            body_len = 1000000;
            ok_on_close = true;    
        }
    }
    else
       body_len = atol(body_length + 16);   // il +16 e' la lunghezza di "Content-Length:"

    log("Calcolato body len di %d bytes\n", body_len);

    len -= header_len;
    char *buffer_ptr = buffer_ + header_len;
    int delta_len = len;


    while (len < body_len) {
        log("Appendo al messaggio %d bytes\n", delta_len);
        if (delta_len > 0)
            body.append(buffer_ptr, delta_len);

        buffer_ptr = buffer_;

        delta_len = recv(sock_, buffer_, 
                std::min<size_t>(body_len - len, sizeof(buffer_) - 1), 0);

        if (delta_len == 0 && ok_on_close) {
            log("Ricevuta terminazione per messaggio dopo %d bytes\n", len);
            body_len = len;
            break;
        }

        if (delta_len < 0 && ERRNO == EWOULDBLOCK) {
            MSleep(10);
            delta_len = 0;
            continue;
        }

        if (delta_len <= 0) {
            log("Ricevuto valore %d da recv, error: %d!\n", delta_len, ERRNO);
            return HTTPMessage::QUIT;
        }

        len += delta_len;
    }

    // gestire la coda del messaggio.
    offset_ = len - body_len;

    if (delta_len) {
        log("Completo il messaggio con %d bytes\n", delta_len);
        body.append(buffer_ptr, delta_len - offset_);
    }

    if (offset_) {
        log("presenti %d bytes in coda al messaggio di %d bytes\n", offset_,
                body_len + header_len);
        memcpy(buffer_, buffer_ptr + delta_len , offset_);
        buffer_[offset_] = 0;
    }

    if (zipped) {
        char window[0x10000];
        z_stream strm;
        std::string source = body;
        std::string dest;
        /* initialize inflate */
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = 0;
        strm.next_in = Z_NULL;
        int ret = inflateInit2(&strm, 47);      /* automatic zlib or gzip decoding */
        strm.avail_out = 0;

        if (ret != Z_OK)  {
            std::cerr << "Unable to initialize zlib decoding!\n";
            return HTTPMessage::QUIT;
        }

        strm.avail_in = source.length();
        strm.next_in = (Bytef*)&source[0]; 
        body.clear();

        do {
            if (strm.avail_out == 0) {                
                strm.avail_out = sizeof(window);
                strm.next_out = (Bytef*)window;
            }

            ret = inflate(&strm, Z_BLOCK);

            body.append(window, sizeof(window) - strm.avail_out);

            if (ret == Z_STREAM_END)
                break;
        }
        while (strm.avail_in > 0);

        inflateEnd(&strm);
    }
    // devo scartare i chunk aggiuntivi
    if (chunked) {
        std::string buffer;
        log("Discarding additional chunks...");
        while (buffer.find("\r\n\r\n") == std::string::npos) {
            char c;
            int rc = ::recv(sock_, &c, 1, 0);
            if (rc <= 0) {
                log("Connection close mentre scarto chunk\n");
                break;
            }
            buffer.push_back(c);
        }
        log("Done, bytes discarded %d\n", buffer.size());
    }

    log("Decodifica completata con successo\n");
    return HTTPMessage::OK;
}
