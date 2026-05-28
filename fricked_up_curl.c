// fricked_up_curl.c
// This file reads newline-separated HTTP URLs from
// a file passed on argv, sends a GET request to each,
// and writes the responses to stdout.
//
// This program will invoke lots of functions you've never seen!
// This is fine. This is what the man pages are for.
// This is what reading code in the real world is like.
//
// Your job is to find as many bugs as possible in this code.
//
// If you ask an LLM, it will probably find some of them pretty quickly.
// This exercise is for you to develop an eye for bugs as a programmer;
// not to achieve some kind of "high score." So please refrain from consulting
// external resources during this exercise. I promise, the man pages have
// all the answers :)

#define _GNU_SOURCE
#include <stdio.h> // for stderr, fprintf, getline
#include <stdlib.h> // for EXIT_FAILURE, exit
#include <stddef.h> // for size_t
#include <strings.h> // for strcasecmp
#include <sys/types.h> // for ssize_t
#include <netdb.h> // for getaddrinfo, freeaddrinfo
#include <sys/socket.h> // for AF_INET, SOCK_STREAM, socket, connect
#include <unistd.h> // for write, close, STDOUT_FILENO
#include <string.h> // for strlen
#include <arpa/inet.h> // for htons

struct url {
    char *scheme;
    char *hostname;
    int port;
    char *path; // without leading '/'
};

struct url parse_url(char *const url) {
        // Here's the plan:
        // We want to break up this URL into 3 parts:
        // - scheme   (the "http" in "http://example.com/cool_path")
        // - hostname (the "example.com" in "http://example.com/cool_path")
        // - path     (the "/cool_path" in "http://example.com/cool_path")
        // Along the way, we also want to collect a port number, if there is one.

        // As we parse through the string, we'll drop null bytes into the middle of it,
        // thereby separating it into these 3 components.

        // Find the ':' that delimits the scheme
        size_t colon_idx = 0;
        while (url[colon_idx] != ':') {
            colon_idx++;
        }

        // Replace it with a null byte,
        // so we can treat the scheme as
        // its own string
        url[colon_idx] = '\0';
        char *const scheme = url;

        // Now, we find the hostname and port

        // Find the end of the hostname
        char *const hostname = url + colon_idx + 1 /* for the '\0' */ + 2 /* for the "//" */;
        size_t end_of_hostname_idx = 0;
        while (hostname[end_of_hostname_idx] != '\0' && hostname[end_of_hostname_idx] != ':' && hostname[end_of_hostname_idx] != '/') {
            end_of_hostname_idx++;
        }

        // After the hostname is either a path, a port number (maybe followed by a path), or nothing
        // Handle the three cases separately.
        int port;
        char *path;
        if (hostname[end_of_hostname_idx] == '\0') {
            // Nothing. Easy
            port = 80;
            path = hostname + end_of_hostname_idx;
        } else if (hostname[end_of_hostname_idx] == '/') {
            // Path
            hostname[end_of_hostname_idx] = '\0'; // null out the '/'
            port = 80;
            path = hostname + end_of_hostname_idx + 1; // skip the null byte
        } else {
            // Port number
            hostname[end_of_hostname_idx] = '\0'; // null out the ':'
            char *port_str = hostname + end_of_hostname_idx + 1; // skip the null byte
            port = 0;
            size_t end_of_port_idx = 0;
            while (port_str[end_of_port_idx] != '\0' && port_str[end_of_port_idx] != '/') {
                port *= 10;
                port += port_str[end_of_port_idx] - '0';
                end_of_port_idx++;
            }
            if (port_str[end_of_port_idx] == '/') {
                // Path
                path = port_str + end_of_port_idx + 1;
            } else {
                // Nothing!
                path = port_str + end_of_port_idx;
            }
        }
        return (struct url){
            .scheme = scheme,
            .hostname = hostname,
            .port = port,
            .path = path
        };
}

void write_all(int const fd, char const *data, size_t len) {
    while (len != 0) {
        ssize_t const write_rc = write(fd, data, len);
        if (write_rc < 0) {
            fprintf(stderr, "write failed\n");
            exit(EXIT_FAILURE);
        } else {
            data += write_rc;
            len -= write_rc;
        }
    }
}

void send_http_request(struct url const url) {
    // You can safely ignore this function in your bug hunt;
    // it's just here to make the program do something cool :)
    // (I'm serious)
    // (But if you're curious and we have time I can explain it to you)

    // Do a DNS lookup for the hostname
    struct addrinfo hints = (struct addrinfo){
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
        .ai_protocol = 0,
        .ai_flags = 0
    };
    struct addrinfo *dns_lookup_result;
    int const gai_rc = getaddrinfo(url.hostname, NULL, &hints, &dns_lookup_result);
    if (gai_rc) {
        fprintf(stderr, "DNS lookup failed!\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in *ipaddr = (struct sockaddr_in *)(dns_lookup_result->ai_addr);

    ipaddr->sin_port = htons((uint16_t)url.port);

    int const sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        fprintf(stderr, "socket failed!\n");
        exit(EXIT_FAILURE);
    }
    if (connect(sock, ipaddr, dns_lookup_result->ai_addrlen)) {
        fprintf(stderr, "connect failed!\n");
        exit(EXIT_FAILURE);
    }

    static char const req_prefix[] = "GET /";
    write_all(sock, req_prefix, sizeof(req_prefix) - 1);
    write_all(sock, url.path, strlen(url.path));
    static char const req_middle[] = " HTTP/1.1\r\nConnection: close\r\nHost: ";
    write_all(sock, req_middle, sizeof(req_middle) - 1);
    write_all(sock, url.hostname, strlen(url.hostname));
    static char const req_end[] = "\r\n\r\n";
    write_all(sock, req_end, sizeof(req_end) - 1);
    
    while (true) {
        char c;
        if (read(sock, &c, 1) != 1) {
            break;
        }
        write(STDOUT_FILENO, &c, 1);
    }

    close(sock);
    freeaddrinfo(dns_lookup_result);
}

int main(int const argc, char const *const *const argv) {
    if (argc == 0) {
        fprintf(stderr, "How did you even do this??\n");
        return EXIT_FAILURE;
    } else if (argc != 2) {
        fprintf(stderr, "Usage: %s <file_with_some_urls>\n", argv[0]);
        return EXIT_FAILURE;
    }

    FILE *const url_file = fopen(argv[1], "r");

    while (true) {
        // Read a line from the file
        char *line = NULL;
        size_t allocation_size;
        ssize_t const getline_rc = getline(&line, &allocation_size, url_file);
        if (getline_rc == -1) {
            // Failed to read a line
            free(line);
            break;
        }

        // Drop the trailing newline,
        // if there is one.
        if (getline_rc > 0 && line[getline_rc - 1] == '\n') {
            line[getline_rc - 1] = '\0';
        }

        struct url url = parse_url(line);
        if (strcasecmp(url.scheme, "http")) {
            fprintf(stderr, "Invalid URL scheme!\n");
            return EXIT_FAILURE;
        }

        send_http_request(url);

        free(line);
    }

    fclose(url_file);
}
