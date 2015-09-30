#include "inet.h"

void error(const char *msg){
    perror(msg);
    exit(0);
}

void createDir(int sockfd){
	printf("~~~  create Directory  ~~~\n\n");

	//Setting directory
	char targetName[256];
	char dir[256] = "/home/";
	
	//get machine current username
	char *username=getenv("USER");
	if(username==NULL){
		//return EXIT_FAILURE;
		exit(0);
	}

	strcat(dir, username);
	char file[256] = "/FRS/Client/";
	strcat(dir, file);

	//Create directory if it does not exist
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}

	DIR *directory;
	struct dirent *ent;
	if((directory = opendir(dir)) != NULL){
		while((ent = readdir(directory)) != NULL){
			printf("%s ", ent->d_name);
		}
		closedir(directory);
		printf("\n");
	}
	else{
		perror("ERROR");
		exit(0);
	}

	char des[256] = "";
	strcat(des, dir);	
	
	printf("Please enter the directory name that you want to create:\n");
	scanf("%s",targetName);
	strcat(dir, targetName);

	if(stat(dir, &st) == -1){
		int status = mkdir(dir, 0700);
		if(status==0){
			printf("Successful create the directory.\n");
		}
		else{
			printf("Cannot create directory and subdirectory in same time.\n");
		}
	}
	else if(stat(dir, &st) == 0){
		printf("\nDirectory already exist\n");
	}

}


void downloadFile(int sockfd){	//Client download file from Server
	printf("Downloading file from Server... ");
	
	int n;
	int buflen;

	//Setting directory
	char revBuff[256];
	char dir[256] = "/home/";
	
	//get machine current username
	char *username=getenv("USER");
	if(username==NULL){
		//return EXIT_FAILURE;
		exit(0);
	}
	
	strcat(dir, username);
	char file[256] = "/FRS/Client/";
	strcat(dir, file);
	//printf("\nLocation: %s", dir);
	
	//Create directory if it does not exist
	struct stat st = {0};
	if(stat(dir, &st) == -1){
		mkdir(dir, 0700);
	}

	//Getting available file from Server
	char tempo[256];
	bzero(tempo,256);
	n = read(sockfd, (char*)&buflen, sizeof(buflen));
	if (n < 0) error("ERROR reading from socket");
	buflen = htonl(buflen);
	n = read(sockfd,tempo,buflen);
	if (n < 0) error("ERROR reading from socket");
	printf("\nAvailable file: \n");
	printf("%s", tempo);

	printf("Please enter the file name that you wanted to download: ");
	char selectFile[256];
	bzero(selectFile,256);
	fgets(selectFile,255,stdin);
    	char input[256];
	
	//Sending file name that Client wants to download to Server
	int datalen = strlen(selectFile);
	int tmp = htonl(datalen);
	n = write(sockfd, (char*)&tmp, sizeof(tmp));
	if(n < 0) error("ERROR writing to socket");
	n = write(sockfd,selectFile,datalen);
	if (n < 0) error("ERROR writing to socket");
	
	char filename[256];
	printf("Save file name as: ");
	fgets(filename, 256, stdin);

	if(filename != NULL){
		strcat(dir, filename);	//Concatenate directory and filename
		//printf("File location: %s", dir);
		printf("Download from Server now ~ %s", filename);

		FILE *fr = fopen(dir, "ab");
		if(fr == NULL){
			printf("File cannot be opened");
			perror("fopen");
			exit(0);
		}
		else{	//Receiving file from Server 
			bzero(revBuff, 256);
			int fr_block_sz = 0;
			while((fr_block_sz = recv(sockfd, revBuff, 256, 0)) > 0){
				int write_sz = fwrite(revBuff, sizeof(char), fr_block_sz, fr);
				if(write_sz < fr_block_sz){
					error("File write failed on server.\n");
				}
				bzero(revBuff, 256);
				if(fr_block_sz == 0 || fr_block_sz != 256){
					break;			
				}
			}
			printf("\nSuccessfully download the file");
			fclose(fr);
		}
	}
	else{
		printf("\nERROR: Filename cannot be NULL");		
		printf("\nERROR: Please try again later");
		exit(0);
	}
}

void sendFile(int sockfd){	//Client send file to Server

	printf("Send file to Server...");
	char buff[256];
	int n;
	
	//Default directory
	char dir[256] = "/home/";
	
	//get machine current username
	char *username=getenv("USER");
	if(username==NULL){
		//return EXIT_FAILURE;
		exit(0);
	}
	
	strcat(dir, username);
	char file[256] = "/FRS/Client/";
	strcat(dir, file);
	
	//Create directory if it does not exist
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}
	
	//Printing files that is available from the directory
	printf("\nAvailable file: \n\n");
	DIR *directory;
	struct dirent *ent;
	if((directory = opendir(dir)) != NULL){
		while((ent = readdir(directory)) != NULL){
			printf("%s ", ent->d_name);
		}
		closedir(directory);
	}
	else{
		perror("ERROR");
		exit(0);
	}
	
	//Selecting file to be sent to Server
	char tempo[256];
	printf("\nPlease enter the file name you want to send: ");
	fgets(tempo, 256, stdin);
	char filename[256];
	strcpy(filename, tempo); 
	
	if(filename != NULL){

		//Sending the file name to Server
		int datalen = strlen(tempo);
		int tmp = htonl(datalen);
		n = write(sockfd, (char*)&tmp, sizeof(tmp));
		if(n < 0) error("ERROR writing to socket");
		n = write(sockfd,tempo,datalen);
		if (n < 0) error("ERROR writing to socket");
	
		char split[2] = "\n";
	 	strtok(tempo, split);

		strcat(dir, filename);
		printf("Sending %s to Server now... ", tempo);
		//printf("\nDir: %s", dir);
	
		FILE *fs = fopen(dir, "rb");	//Read file
		if(fs == NULL){
		  printf("\nERROR: File not found.\n");
		  perror("fopen");
		  exit(0);
		}
		else{	//Sending file to Server
			bzero(buff, 256);
			int fs_block_sz;
			while((fs_block_sz = fread(buff, sizeof(char), 256, fs)) > 0){
				if(send(sockfd, buff, fs_block_sz, 0) < 0){
					fprintf(stderr, "ERROR: Failed to send file. %d", errno);
					break;
				}
				bzero(buff, 256);
			}
			printf("\nSuccessfully send the file!");
			fclose(fs);
		}
	}
	else{
		printf("\nERROR: Filename cannot be NULL");		
		printf("\nERROR: Please try again later");
		exit(0);
	}
		
}

void deleteDir(int sockfd){
	printf("~~~  Delete Directory  ~~~\n\n");

	//Setting directory
	char targetName[256];
	char dir[256] = "/home/";
	
	//get machine current username
	char *username=getenv("USER");
	if(username==NULL){
		//return EXIT_FAILURE;
		exit(0);
	}

	strcat(dir, username);
	char file[256] = "/FRS/Client/";
	strcat(dir, file);

	//Create directory if it does not exist
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}

	system("ls -D FRS/Client");
	printf("~ File with extension .txt? is not directory, it won't able to delete. ~\n\n");

	printf("Please enter the directory name that you want to delete:\n");
	scanf("%s",targetName);
	
	strcat(dir, targetName);
	if(stat(dir, &st) == -1){
		printf("\nInvalid directory name\n");
	}
	else if(stat(dir, &st) == 0){
		char confirmation[3];
		printf("Are you sure want to delete this directory?(y/n)\n");
		fflush (stdout);
		scanf("%s",confirmation);
	
		if(strcmp(confirmation,"y") == 0){
			rmdir(dir);
			printf("\nSuccessful delete the directory");
		}else{
			printf("\nYou have cancel the delete action");
		}
	}

}

void copyDir(int sockfd){
  	printf("~~~  Copy Directory  ~~~\n\n");

	//Setting directory
	char targetName[256];
	char dir[256] = "/home/";
	
	//get machine current username
	char *username=getenv("USER");
	if(username==NULL){
		//return EXIT_FAILURE;
		exit(0);
	}

	strcat(dir, username);
	char file[256] = "/FRS/Client/";
	strcat(dir, file);

	//Create directory if it does not exist
	struct stat st = {0};
	if(stat(dir, &st) == -1){
	  mkdir(dir, 0700);
	}

	DIR *directory;
	struct dirent *ent;
	if((directory = opendir(dir)) != NULL){
		while((ent = readdir(directory)) != NULL){
			printf("%s ", ent->d_name);
		}
		closedir(directory);
		printf("\n");
	}
	else{
		perror("ERROR");
		exit(0);
	}

	char des[256] = "";
	strcat(des, dir);	
	
	printf("Please enter the directory name that you want to copy:\n");
	scanf("%s",targetName);
	strcat(dir, targetName);
	
	if(stat(dir, &st) == -1){
		printf("\nInvalid directory name\n");
	}
	else if(stat(dir, &st) == 0){

		char desName[256];
		printf("\nPlease enter the directory name that you want to paste:\n");
		scanf("%s",desName);
		strcat(des, desName);
		
		if(stat(dir, &st) == -1){
			printf("\nDirectory don't exist\n");
		}
		else{
			char commandAction[256] = "cp -r ";
			strcat(commandAction, dir);
			strcat(commandAction, " ");
			strcat(commandAction, des);

			system(commandAction);
			printf("\nSuccessful copy the directory");
		}
	}
}

int main(int argc, char *argv[])	//Connecting to Server (SOCKET)
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
		(char *)&serv_addr.sin_addr.s_addr,
		server->h_length);
    serv_addr.sin_port = htons(portno);
    
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");

    printf("\n~~ Connected to FRS Server! ~~");

    int count = 0;
    while(count == 0){	//Getting Client's choice
	
		//printf("\n\n1.Create 2.Download 3.Send 4.Delete 5.Exit : ");
		printf("\n\nFile:      1.Send     2.Download");
		printf("\nDirectory: 3.Create   4.Delete     5.Copy     6.Exit : ");
		char buffer[256];
		printf("\nPlease insert your choice: ");
		bzero(buffer,256);
		fgets(buffer,255,stdin);
		char input[256];
		strcpy(input, buffer);
		
		//Sending Client's choice to Server
		int datalen = strlen(buffer);
		int tmp = htonl(datalen);
		n = write(sockfd, (char*)&tmp, sizeof(tmp));
		
		if(n < 0) error("ERROR writing to socket");
		
		n = write(sockfd,buffer,datalen);
		
		if (n < 0) error("ERROR writing to socket");
		

		if((strcmp(input, "1\n")) == 0){	//Client send file to Server
		   sendFile(sockfd);
		   count = 0;
		}
		else if((strcmp(input, "2\n")) == 0){	//Client download file from Server
		   downloadFile(sockfd);
		   count = 0;
		}
		else if((strcmp(input, "3\n")) == 0){	//Client send file to Server
		   //createFile(sockfd);
		   createDir(sockfd);
		   count = 1;
		}
		else if((strcmp(input, "4\n")) == 0){	//Delete directory on client-site
		   deleteDir(sockfd);
		   count = 1;
		}
		else if((strcmp(input, "5\n")) == 0){	//Copy directory on client-site
		   copyDir(sockfd);
		   count = 1;
		}
		else if((strcmp(input, "6\n")) == 0){	//Client disconnect from Server
		   count++;
		}
		else{
		   printf("\nWrong input, please try again.");	//Invalid input from Client
		   count = 0;
		}
    }
    close(sockfd);
    printf("\n~~ Process End.\n~~ Disconnected from the Server.\n\n");
    return 0;
}
