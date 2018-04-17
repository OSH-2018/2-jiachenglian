#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

struct cmd
{
    int argc;  //一个管道参数个数
    char *argv[128];//命令参数
}*cmdlist[10];

void execute_pipe(int index, int num)
{
    
    
    if (index == num - 1)
        execvp(cmdlist[index]->argv[0], cmdlist[index]->argv);
    int fd[2];
    pipe(fd);//创建管道
	pid_t pid = fork();
    if (pid == 0)
    {
        dup2(fd[1], STDOUT_FILENO);
        close(fd[0]);
        close(fd[1]);
        execvp(cmdlist[index]->argv[0], cmdlist[index]->argv);
    }
    dup2(fd[0], STDIN_FILENO);
    close(fd[0]);
    close(fd[1]);
	int exitstatus;
	while(wait(&exitstatus)!=pid);
    execute_pipe(index + 1, num);
}

int main() {
    /* 输入的命令行 */
    char cmd[256];
    /* 命令行拆解成的各部分，以空指针结尾 */
    char *args[128];
    while (1) {
        /* 提示符 */
        printf("# ");
        fflush(stdin);
        fgets(cmd, 256, stdin);
        /* 清理结尾的换行符 */
        int i;
        for (i = 0; cmd[i] != '\n'; i++)
            ;
        cmd[i] = '\0';
        /* 拆解命令行 */
        args[0] = cmd;
        for (i = 0; *args[i]; i++)
            for (args[i+1] = args[i] + 1; *args[i+1]; args[i+1]++)
                if (*args[i+1] == ' ') {
                    *args[i+1] = '\0';
                    args[i+1]++;
                    break;
                }
        args[i] = NULL;
        
        /* 没有输入命令 */
        if (!args[0])
            continue;
        
        /* 内建命令 */
        if (strcmp(args[0], "cd") == 0) {
            if (args[1])
                chdir(args[1]);
            continue;
        }
        if (strcmp(args[0], "pwd") == 0) {
            char wd[4096];
            puts(getcwd(wd, 4096));
            continue;
        }
        if (strcmp(args[0], "exit") == 0)
            return 0;
        
        if (strcmp(args[0], "export") == 0)
        {
            for (i=0; args[1][i] != '='; i++);
            args[1][i] = '\0';
            args[2] = args[1]+i+1;
            if (setenv(args[1], args[2], 1) == 0)
                printf("success\n");
            else printf("failed\n");
            continue;
        }
        
        /* 外部命令 */
        
        int num = 0;//管道数
        for (int i=0; args[i]; i++)
        {
            
            cmdlist[num] = (struct cmd*)malloc(sizeof(struct cmd));
            cmdlist[num]->argc = 0;
            memset(cmdlist[num]->argv, 0, sizeof(char*)*128);
            for (int j=0; args[i]&&(strcmp(args[i], "|") != 0); i++,j++)
            {
                cmdlist[num]->argc++;
                cmdlist[num]->argv[j] = (char*)malloc(sizeof(char)*strlen(args[i]));
                strcpy(cmdlist[num]->argv[j], args[i]);
            }
            num++;
            if (!args[i]) break;
        }
        
        pid_t pid = fork();
        if (pid == 0) {
            /* 子进程 */
            execute_pipe(0, num);
            /* execvp失败 */
            return 255;
        }
        /* 父进程 */
        wait(NULL);
    }
}
