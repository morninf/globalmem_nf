#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>

#define CMD_BUFFER_LEN	100

static int str_to_num(char *dat)
{
	int len = strlen(dat);
	int ret = 0;
	int mid_dat = 0;
	int pow = 1;
	int i,j=0;
	if(len > 6 || len < 0 )
		return -1;
	for(i =0; i<len; i++)
	{
		mid_dat = dat[len-1-i]-0x30;
		if(mid_dat >= 0 && mid_dat <= 9)
		{
			pow = 1;
			for(j=0;j<i;j++)
			{
				pow *= 10;
			}
			ret += pow*mid_dat;
		}
		else if( mid_dat == (0x2D-0x30) && i == len-1)
			ret = 0-ret;
		else
			return -2;
	}
	return ret;
}


/*	"read" "pos" "len"	name: 1*/
/*	"write" "pos" "len"		2*/
/*	"read" "pos" "len"	*/
static int cmd_prase(char *cmd, int *op_pos, int *op_len)
{
	char cmd_data[5][10];
	int i,idx=0;
	int j=0;
	int ret=0;
	int cmd_len = strlen(cmd);
	for(i=0;i<cmd_len;i++)
	{
		if(cmd[i] == ',')
		{
			cmd_data[idx++][j] = '\0';
			j=0;
			continue;
		}
		cmd_data[idx][j++] = cmd[i];
	}
	cmd_data[idx][j] = '\0';
	
	if( strcmp("read",cmd_data[0]) == 0)
	{
		if(idx == 2)
		{
			*op_pos = str_to_num(cmd_data[1]);
			*op_len = str_to_num(cmd_data[2]);
			ret = 1;
			//printf("pos: %d, len: %d\n",fp_pos,fp_len);
		}
	}
	else if( strcmp("write",cmd_data[0]) == 0)
	{
		if(idx == 2)
		{
			*op_pos = str_to_num(cmd_data[1]);
			*op_len = str_to_num(cmd_data[2]);
			ret = 2;
		}
		
	}
	else if( strcmp("quit",cmd_data[0]) == 0)
	{
		ret = -1;
	}
	else
	{
		printf("invaild command\n");
		ret = 0;
	}
	return ret;
}


int main(int argc, char *argv)
{
	int fd;
	int i=0;
	int count=0;
	char cmd_buf[CMD_BUFFER_LEN];
	int op_name=0;
	int op_pos = 0;
	int op_len = 0;
	char read_buf[4096];
	char write_buf[4096];
	fd = open("/dev/globalmem",O_RDWR);
	if(fd<0)
	{
		printf("open device file fail\n");
		return 0;
	}
	printf("open file success\n");
	while(1)
	{
		printf("cmd->");
		scanf("%s",cmd_buf);
		op_name = cmd_prase(cmd_buf,&op_pos,&op_len);
		printf("pos: %d\n",op_pos);
		if(op_name == 0)
		{
			;
		}
		else if( op_name == 1)
		{
			count = read(fd,read_buf,op_len,op_pos);
			if(count<0)
				printf("read error\n");
			else
			{
				printf("read success: %d\n",count);
				for(i=0;i<count;i++)
					printf("%c",read_buf[i]);
				printf("\n");
			}
		}
		else if( op_name == 2)
		{
			printf("input write chars\n");
			scanf("%s",write_buf);
			printf("input char: %s\n",write_buf);
			count = write(fd,write_buf,strlen(write_buf),op_len);
			if(count>0)
				printf("write success: %d\n",count);
		}
		else
		{
			break;
		}
	}
	close(fd);
	return 0;
}
