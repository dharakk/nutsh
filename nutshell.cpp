#include<stdio.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<stdlib.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>

#define MAX_SIZE 1024
#define KCYN  "\x1B[36m"
#define RESET   "\x1b[0m"

char prompt[MAX_SIZE],history[36000][MAX_SIZE],*home;
int gi=0,lim=0;

void initializePrompt(){
	getcwd(prompt,1023);
}

void signal_handler(int sig){ 
	//avada kedavra!!! :-P 
}

void handleBang(char *input){

	char temp[MAX_SIZE],*token,bang[255];
	int flag=0,index;
	token=strtok(input,"|");

	strcpy(temp," ");
	
	flag=0;
	while(token!=NULL){
		
		int i=0;
		while(token[i]==' ' || token[i]=='\t') i++;
		if(token[i]=='!'){
			i++;
			int k=0;
			while(token[i]!=' ' && token[i]!='|' && token[i]!='>' && token[i]!='<' && token[i]!='\0'){
				bang[k]=token[i];
				k++;
				i++;
			}
			bang[k]='\0';
			if(atoi(bang)!=0){
				int x= atoi(bang);
				if(x<0) index=lim+x;
				else index=(x%lim)-1;
		
			}
			else{
				int i=lim-1;
				for(;i>=0;i--){
					char *tmp=strstr(history[i],bang);
					if(tmp!=NULL && strlen(tmp) == strlen(history[i])){
						index=i;
						break;
					}
				}
			}
			if(flag==0){
				strcat(temp,history[index]);
				flag=1;
			}
			else{
				strcat(temp," | ");
				strcat(temp,history[index]);
			}
		}
		else{
			if(flag==0){
				strcat(temp,token);
				flag=1;
			}
			else{
				strcat(temp," | ");
				strcat(temp,token);
			}
		}
		token=strtok(NULL,"|");
	}
	strcpy(input,temp);
	printf(" %s \n",input);
}

void sethistorypath(){
	char *str=getenv("HOME"); home=(char*)malloc(sizeof(char)*255);
	strcpy(home,str);
	home=strcat(home,"/");
	home=strcat(home,".sh_history");
}

void initializeHistory(){
	FILE *fp;

	sethistorypath();
      
    fp = fopen(home, "r");
    if (fp != NULL) {
       while(!feof(fp)) {
   			if (fgets(history[lim],MAX_SIZE,fp)) {
   				 char *newline = strchr(history[lim], '\n');
				if ( newline != NULL ){
					*newline = '\0'; 
				}
       			 lim++;
  			}
		}
		  fclose(fp);
    }
  
}

void addCommand(char* comm){
	strcpy(history[lim],comm);
	lim++;
	FILE *fp;
     fp = fopen(home, "a");
     if (fp != NULL) {
      	 fprintf(fp, "%s\n", comm);
      } 
  	fclose(fp);

}

void printHistory(){
	int i;
	for(i=0;i<lim;i++){
		printf(" %4d %s\n",i+1,history[i]);
	}
}

void getCommand(char **input,char **arglist,char *mode){
	int k=0,j=0,flag=0;

	char *temp=*input;
	while(temp[gi]!='\0' && temp[gi]!='|' && temp[gi]!='<' && temp[gi]!='>'){

		flag=0;

		if(temp[gi]==' '){
			if(gi!=0){
				arglist[j][k]='\0';
				j++;
				k=0;
			}
			while(temp[gi]==' '){
				gi++;
			}
			flag=1;
		}
		else{
			if(temp[gi]=='\"' || temp[gi]=='\''){
				char term=temp[gi];
				gi++;
				//printf("here");
				while(temp[gi]!=term){
					arglist[j][k]=temp[gi];
					gi++;
					k++;
				}
				gi++;
				flag=2;

			}
			else{
				arglist[j][k]=temp[gi];
				gi++;
				k++;

			}
			flag=2;
		}
	}

	*mode=temp[gi];

	if(temp[gi] != '\0')
		gi++;

	while(temp[gi] != '\0' && temp[gi]==' '){
				gi++;
	}

	if(flag==1)
		arglist[j]=NULL;
	if(flag==2){
		arglist[j][k]='\0';
		arglist[++j]=NULL;
	}

}


void execute(char *input){
	
	char **arglist,**arglist2,*token,mode;
	int pid,isPiped=0,ind=0,outd=1,pip[2],firstPipe=0,status;
	mode ='d';
	char *temp=strchr(input,'|');
	if(temp!=NULL){
		
		isPiped=1;
	}

	while(1){

		if(isPiped==1){
			pipe(pip);
		}

		pid=fork();

		if(pid<0){
			printf("Error occured!! :-( \n");
		}
		else if(pid==0){
			
			while(1){
				
				arglist2=(char**)malloc(sizeof(char*)*3);
				for(int x=0;x<10;x++)
					arglist2[x]=(char*)malloc(sizeof(char)*80);
			
				if(mode=='|'){
					outd=pip[1];
					mode='d';
					break;
				}
				if(mode=='d'){
					arglist=(char**)malloc(sizeof(char*)*10);
					for(int x=0;x<10;x++){
						arglist[x]=(char*)malloc(sizeof(char)*80);
					}
					getCommand(&input,arglist,&mode);
				}
				else if(mode=='<'){
						
					getCommand(&input,arglist2,&mode);
					int fdi=open(arglist2[0],O_RDONLY);
					if(fdi<0){
						printf("File does not exist!! :-( \n");
							exit(1);
					}
					ind=fdi;
				}
				else if(mode=='>'){
						
					getCommand(&input,arglist2,&mode);
					int fdo=open(arglist2[0],O_CREAT | O_WRONLY ,0777);
					
					outd=fdo;
				}
				else if(mode=='\0'){					
					break;
				}
			}
			if(ind!=0){
				dup2(ind,0);
				close(ind);
			}
			if(outd!=1){
				dup2(outd,1);
				close(outd);
			}
			if(strcmp(arglist[0],"cd")==0){
				
				exit(0);
			}
			else if(strcmp(arglist[0],"pwd")==0){
				printf("%s\n",prompt);
				exit(0);
			}
			else if(strcmp(arglist[0],"history")==0){
				addCommand(input);
				printHistory();
				exit(0);
			}
			else if(strcmp(arglist[0],"echo")==0 && arglist[1][0] == '$'){
				char *var=arglist[1];
				var++;
				char *val=getenv(var);
				printf("%s\n",val);
				exit(0);
			}
			else if(execvp(arglist[0],arglist) == -1){
				printf("execution of command failed\n");
				exit(1);
			}
		}
		else{
			int flag=0;
			char mode2,**arglist3;
			if(isPiped==1){
				
				firstPipe=1;
				mode2='d';
				flag=0;

				while(mode2!='|' && mode2!='\0'){
					arglist3=(char**)malloc(sizeof(char*)*10);
					for(int x=0;x<10;x++){
						arglist3[x]=(char*)malloc(sizeof(char)*80);
					}
					getCommand(&input,arglist3,&mode2);
					if(strcmp(arglist3[0],"cd")==0){
						chdir(arglist3[1]);
						initializePrompt();
						flag=1;
					}
				}
				if(mode2!='\0'){
					close(pip[1]);
					if(flag==0)
						ind=pip[0];
					else flag=0;
					while(wait(&status) != pid);
					continue;
				}
				close(pip[0]);
			}
			arglist3=(char**)malloc(sizeof(char*)*10);
					for(int x=0;x<10;x++){
						arglist3[x]=(char*)malloc(sizeof(char)*80);
					}
					getCommand(&input,arglist3,&mode2);
					if(strcmp(arglist3[0],"cd")==0){
						chdir(arglist3[1]);
						initializePrompt();
						flag=1;
			}
			while(wait(&status) != pid);
			break;
		}
	}
	addCommand(input);
}

int main(int argc,char **argv){
	initializePrompt();
	initializeHistory();
	signal(SIGINT, signal_handler); printf("\n");	

	while(1){
		printf(KCYN "My_Shell:%s$> " RESET,prompt);
		char *input;

		size_t len = MAX_SIZE;

		input=(char*)malloc(sizeof(char)*MAX_SIZE);
		if(getline(&input,&len,stdin) != -1){
			 char *newline = strchr(input, '\n');
				if ( newline != NULL ){
					*newline = '\0'; 
				}
		}
		if(strcmp(input,"exit")==0){
			exit(0);
		}
		if(strstr(input,"!")!=NULL){
			handleBang(input);
		}
		gi=0;
		if(strlen(input)>0)
			execute(input);

	}
	return 0;
}

