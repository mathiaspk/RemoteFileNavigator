struct client{
	struct client *next;
	int socket;
	char *current_directory;
};
typedef struct client CLIENT;
CLIENT *head;

char root[PATH_MAX];

char* findDirectory(int socket);
int setDirectory(int socket, char *new_dir);

int cat_file(char file[], int socket);
int file_menu(int socket, int id);
int get_file_info(char file[], int socket);
int change_directory(char command[], int socket);
int cd_submenu(int socket);
int getpwd(int socket);
int getLS(int socket);
int command_intrp(char command[], int socket);
int ls_exec(char command[], int socket);
void printmenu(void);
int send_info(int socket, char *command);
int rec_info(int socket, int id);

