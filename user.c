#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "user.h"
#include "avl.h"

typedef void *(*init_functions)();
typedef int (*compare_function)(const void *, const void *, const void *);

typedef enum { id, login, type, created_at, followers, follower_list, following, following_list, public_gists, public_repos } TYPE_USER;

typedef enum { repo_id, author_id, committer_id, commit_at, message } TYPE_COMMIT;

typedef enum { usercsv = 10, reposcsv = 15, commitcsv = 5 } FILE_OPENED;

typedef enum { year, month, day, hours, minutes, seconds } DATE;

typedef struct line_commit{
    int repo_id;
    int author_id;
    int committer_id;
    char *commit_at;
    char *message;
} *CommitP;

#define PASSSTRUCT(elemento)\
    user->elemento = strdup(list_tokens[elemento]);

int test_integer(char *number){
    char *resto;
    int int_val = strtol(number, &resto, 10); 
    return !(int_val < 0 && (strlen(resto) != 0 && resto[0] != '\n'));
}


int isDateValid(int date[]){
    int r;
    if(date[year] == 2021){
        if(date[month] == 11){
            r = date[day] <= 7;
        }
        else{
            r = date[month] < 11;
        }
    }
    else if(date[0] == 2005){
        if(date[month] == 4){
            r = date[day] > 7;
        }
        else{
            r = date[month] > 4;
        }
    }
    else{
        r = date[year] > 2005 && date[year] < 2021;
    }
    return r;
}
    
int test_date(char *date){
    int r = 0;
    int dateElems[6];
    r = (sscanf(date, "%d-%d-%d %d:%d:%d", &dateElems[year], &dateElems[month], &dateElems[day], &dateElems[hours], &dateElems[minutes], &dateElems[seconds]) == 6
            && isDateValid(dateElems));
    return r;
}

int test_integers(char **list_tokens, int file_opened){
    int r = 0;
    if(file_opened == usercsv){
        r = test_integer(list_tokens[public_repos]) && 
            test_integer(list_tokens[id]) && 
            test_integer(list_tokens[followers]) && 
            test_integer(list_tokens[public_gists]) && 
            test_integer(list_tokens[following]);
    }
    else if(file_opened == commitcsv){
        r = test_integer(list_tokens[repo_id]) &&
            test_integer(list_tokens[committer_id]) &&
            test_integer(list_tokens[author_id]);
    }

    return r;
}

int test_followerList(char *list_token, int number_foll){
    int r = 1;
    char *token = NULL;
     
    char *list_tokens = strdup(list_token);
    char *tofree = list_tokens;
    token = strsep(&list_tokens, "[, ]");
    int test_numbers = 0;
    while(r == 1 && token != NULL){
        if(strlen(token) != 0 && atoi(token) < 0){
            r = 0;
        }
        token = strsep(&list_tokens, "[, ]");
        test_numbers++;
    }
    r = r && (test_numbers - 3 == number_foll);
    free(tofree);
    return r; 
}

int test_lists(char *list_tokens[]){
    int r = 1;

    r = test_followerList(list_tokens[follower_list], atoi(list_tokens[followers])) 
                         && test_followerList(list_tokens[following_list], atoi(list_tokens[following]));
    return r;
} 

int test_emptyData(char **list_tokens, int file_opened){
    int r = 1;
    int i = 0;
    while(r == 1 && i < file_opened){
        if(strlen(list_tokens[i]) == 0){
            r = 0;
        }
        i++;
    }
    return r;
}

int test_type(char *type){
    int r;
    r = strcmp(type, "Bot") == 0 || strcmp(type, "Organization") == 0 || strcmp(type, "User") == 0; 
    return r;
}

void passToStruct(char *list_tokens[], void *line, int file_opened){
    if(file_opened == usercsv){
        UserP user = (UserP)line;
        user->public_repos = strdup(list_tokens[public_repos]);
        user->id = atoi(list_tokens[id]);
        user->following = strdup(list_tokens[following]);
        user->followers = strdup(list_tokens[followers]);
        user->follower_list = strdup(list_tokens[follower_list]);
        user->following_list = strdup(list_tokens[following_list]);
        user->type = strdup(list_tokens[type]);
        user->login = strdup(list_tokens[login]);
        user->created_at = strdup(list_tokens[created_at]);
        user->public_gists = strdup(list_tokens[public_gists]);
    }
    else if(file_opened == commitcsv){
        CommitP commit = (CommitP)line;
        commit->repo_id = atoi(list_tokens[repo_id]);
        commit->author_id = atoi(list_tokens[id]);
        commit->committer_id = atoi(list_tokens[committer_id]);
        commit->commit_at = strdup(list_tokens[commit_at]);
        commit->message = strdup(list_tokens[message]);
    }
}

int is_line_ok(char **list_tokens, int file_opened){
    int r = 0;
    if(file_opened == usercsv) r = test_emptyData(list_tokens, file_opened) && 
                                   test_type(list_tokens[type]) && 
                                   test_date(list_tokens[created_at]) && 
                                   test_integers(list_tokens, file_opened) && 
                                   test_lists(list_tokens);

    else if(file_opened == commitcsv) {
        r = test_emptyData(list_tokens, file_opened) &&
            test_integers(list_tokens, file_opened) &&
            test_date(list_tokens[commit_at]);
    }
    return r;
}

void free_tokens(char *list_tokens[], int type){
    int N = 0;
    if(type == usercsv) N = 10;
    else if (type == reposcsv) N = 15;
    else N = 5;
    for(int i = 0; i < N; i++){
        free(list_tokens[i]);
    }
}

void write_line(char *line, int file_opened){
    FILE *file;
    if(file_opened == usercsv){
        file = fopen("./saida/users.csv", "a");
        if(!file){
            perror("users_ok.csv");
        }
        fputs(line, file);
        fclose(file);
    }
    else{
        file = fopen("./saida/commits.csv", "a");
        if(!file){
            perror("commits.csv");
        }
        fputs(line, file);
        fclose(file);
    }

}

void *init_commit(){
    CommitP new = (CommitP)malloc(sizeof(struct line_commit));
    if(!new){
        perror("Init commit");
    }
    return new;
}

void *init_user(){
    UserP new = (UserP)malloc(sizeof(struct line));
    if(new == NULL){
        perror("New user");
    }
    return new;
}

char **list_tokens_init(int file_opened){

    char **list = (char **)malloc(sizeof(char *) * file_opened);
    if(list == NULL){
        perror("list");
    }
    return list;
}

void *init_repos(){
    
}

int compare_user(const void *a, const void *b, void *param){
    const UserP u1 = (UserP)a;
    const UserP u2 = (UserP)b;

    return u2->id - u1->id;
}

int compare_repos(const void *a, const void *b, void *param){

}

int compare_commit(const void *a, const void *b, void *param){
    const CommitP c1 = (CommitP)a;
    const CommitP c2 = (CommitP)b;
    return c2->repo_id - c1->repo_id;
}

struct avl_table *insert(void *user, struct avl_table *tree){
    avl_insert(tree, (UserP)user);
    return tree; 
}

struct avl_table *parse(char *buffer, int file_opened){
    char *token = NULL;

    //keep the line
    char *line = strdup(buffer);
    char **list_tokens = NULL;
    list_tokens = list_tokens_init(file_opened);

    int type = 0;
    token = strsep(&buffer, ";\n");
    while(buffer != NULL){
        list_tokens[type] = strdup(token);
        type++;
        token = strsep(&buffer, ";\n");
    }
    init_functions init[] = { init_commit, init_user, init_repos };
    avl_comparison_func (*comp[]) = {compare_commit, compare_user, compare_repos};

    struct avl_table *tree = NULL;

    void *user;
    if(is_line_ok(list_tokens, file_opened)){
        user = init[(file_opened / 5) - 1]();
        write_line(line, file_opened);
        passToStruct(list_tokens, user, file_opened);
        tree = avl_create((comp[(file_opened / 5) - 1]), NULL, NULL);
        tree = insert(user, tree);
    }
        free(line);
        free_tokens(list_tokens, file_opened);
        free(list_tokens);
        return tree;
}

struct avl_table *read_lines(FILE *file, int file_opened){
    char *buffer = NULL;
    //skip first line
    char line_1[200];
    fgets(line_1, 200, file);

    write_line(line_1, file_opened);

    size_t linecapp = 0;

    struct avl_table *tree = NULL;

    while(getline(&buffer, &linecapp, file) != -1){
        tree = parse(buffer, file_opened);
    }
    free(buffer);

    return tree;
}

static void
print_tree_structure (const struct avl_node *node, int level)
{
  /* You can set the maximum level as high as you like.
     Most of the time, you'll want to debug code using small trees,
     so that a large |level| indicates a ``loop'', which is a bug. */
  if (level > 16)
    {
      printf ("[...]");
      return;
    }

  if (node == NULL)
    return;

  printf ("%d",  ((UserP)node->avl_data)->id);
  if (node->avl_link[0] != NULL || node->avl_link[1] != NULL)
    {
      putchar ('(');

      print_tree_structure (node->avl_link[0], level + 1);
      if (node->avl_link[1] != NULL)
        {
          putchar (',');
          print_tree_structure (node->avl_link[1], level + 1);
        }

      putchar (')');
    }
}

/* Prints the entire structure of |tree| with the given |title|. */
void
print_whole_tree (const struct avl_table *tree, const char *title)
{
  printf ("%s: ", title);
  print_tree_structure (tree->avl_root, 2);
  putchar ('\n');
}

int main(int argc, char *argv[]){
    FILE *file = NULL; 
    file = fopen("./entrada/users.csv", "r");
    if(!file){
        perror("users.csv");
    }

    struct avl_table *user = read_lines(file, usercsv);    
    print_whole_tree(user, "Users:");
    fclose(file);

    file = fopen("./entrada/commits.csv", "r");
    if(!file){
        perror("commits.csv");
    }
    struct avl_table *commit = read_lines(file, commitcsv);
    fclose(file);


    return 0;
}
