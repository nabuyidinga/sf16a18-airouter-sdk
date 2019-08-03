#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#define BUF_SIZE 1024
#define KEY "LOC"
#define KEY_LEN 4
#include <sys/types.h>
#include <regex.h>
#include <assert.h>

static char* substr(const char*str, unsigned start, unsigned end) 
{
	unsigned n = end - start; 
	static char stbuf[256]; 
	strncpy(stbuf, str + start, n); 
	stbuf[n] = 0;  
	return stbuf; 
}
int a[4];
int main()
{
	int b[4];
	interrupt();
	b[0]=a[0];
	b[1]=a[1];
	b[2]=a[2];
	b[3]=a[3];
	sleep(1);
	interrupt();
	printf("in 1 sec:cpu0 interrput num:%d\ncpu1 interrupt num:%d\ncpu2 interrupt num:%d\ncpu3 interrupt num:%d\n",a[0]-b[0],a[1]-b[1],a[2]-b[2],a[3]-b[3]);
	printf("TPASS:      having calculate cpu interrupts number\n");
}
int interrupt()		
{
	int ch = 0;
	int first = 1;
	int flag = 0;
	int count = 0;
	int pre_pos = 0, cur_pos = 0;
	char buf[BUF_SIZE] = {0};
	FILE *fp = NULL;
	fp = fopen("/proc/interrupts", "r");
	while ((ch = fgetc(fp)) != EOF)
	{
		if (ch == '\n')
		{
			count++;//遇到'\n'的个数 
			pre_pos = cur_pos;//上次遇到'\n'时文件指针的位置
			cur_pos = ftell(fp);//当前遇				
			memset(buf, 0, sizeof(buf));
			fgets(buf, KEY_LEN, fp);
			if (strcmp(buf, KEY) == 0)
			{
				fseek(fp, (-1) * (KEY_LEN - 1), SEEK_CUR);
				memset(buf, 0, sizeof(buf));
				fgets(buf, cur_pos-1-pre_pos, fp);
				printf("%s\n", buf);
				const char *pattern = "([^0-9]*)([0-9]+)([^0-9]*)([0-9]+)([^0-9]*)([0-9]+)([^0-9]*)([0-9]+)([^0-9]*)?";
				const char *txt1=buf;
				regex_t regex;
				int eflags = 0;
				int errcode = regcomp(&regex, pattern, REG_EXTENDED);
				printf("regcomp code=%d\n",errcode);
				regmatch_t value[11];
				errcode = regexec(&regex, txt1, 11, value, eflags);
				printf("regexe code=%d\n",errcode);
				assert(0 == errcode); //match success 
				int x;
				int i=0;
				for (x = 0; x < 10; ++ x) {
					if( value[x].rm_so == -1) continue;
					char *str = substr(txt1, value[x].rm_so, value[x].rm_eo);
					printf("  $%d='%s'---length=%d\n", x, str,strlen(str)); 
					if(x==2 || x==4 || x==6 || x==8){
						a[i]=atoi(str);
						i++;
					}
				}
			}
		}
	}
}
