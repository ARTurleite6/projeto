#ifndef user_h
#define user_h

typedef struct line{
    char *public_repos;
    int id;
    char *followers;
    char *public_gists;
    char *following;
    char *follower_list;
    char *following_list;
    char *created_at;
    char *login;
    char *type;
} User, *UserP;

#endif
