#ifndef _BSD_STRUCT_HOSTENT_H_
#define _BSD_STRUCT_HOSTENT_H_

struct hostent {
    char  *h_name;      /* Official name of the host. */
    char **h_aliases;   /* A pointer to an array of pointers to alternative host names,
                             terminated by a null pointer. */
    int    h_addrtype;  /* Address type. */
    int    h_length;    /* The length, in bytes, of the address. */
    char **h_addr_list; /* A pointer to an array of pointers to network addresses (in
                             network byte order) for the host, terminated by a null pointer. */
    char  *h_addr;
};

#endif
