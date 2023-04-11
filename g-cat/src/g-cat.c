#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <unistd.h>

void usage();
void print(char * buf, int enter, int tab, int number);

int main(int argc, char *argv[]){
	int enter = 0, tab = 0, number = 0, use = 1;
	int c;

	while((c = getopt(argc,argv,"ETn")) != -1){
		switch(c){
			case 'E':
				enter = 1;
				break;
			case 'T':
				tab = 1;
				break;
			case 'n':
				number = 1;
				break;
			default:
				use = 0;
				break;
		}
	}
	if(!use){
		usage();
		return 0;
	}
	for(int i = optind;i < argc;i++){
		print(argv[i],enter,tab,number);
	}
	
	return 0;
}

void usage(){
	printf("Usage: g-cat [OPTION] [FILES]\n" 
           "  -E                display $ at end of each line\n" 
           "  -T                display TAB characters as ^I\n" 
           "  -n                number all output lines\n"); 
}

void print(char * buf, int enter, int tab, int number){
	int fd, size, end_of_line = 0, line = 1;;
	char c, line_c;
	fd = open(buf,O_RDONLY);

	if(fd == -1){
		printf("g-cat: %s: No such file or directory\n",buf);
		return ;
	}

	size = lseek(fd, 0, SEEK_END);
	end_of_line = 1;

	for(int i = 0 ; i < size ; i++){
		lseek(fd,i,SEEK_SET);
		read(fd,&c,1);		
		if(number && end_of_line){
			line_c = line + '0';
			write(STDOUT_FILENO,"     ",5);
			write(STDOUT_FILENO,&line_c,1);
			write(STDOUT_FILENO,"   ",2);
			end_of_line = 0;
			line++;
		}

		if(c == 10){
			if(enter) {
				write(STDOUT_FILENO,"$",1);
			}
			write(STDOUT_FILENO,&c,1);
			end_of_line = 1;
		}else if(tab && c == 9){
			write(STDOUT_FILENO,"^I",2);
		}else{
			write(STDOUT_FILENO,&c,1);
		}
	}
	close(fd);
}
