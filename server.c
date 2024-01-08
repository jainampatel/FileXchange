// Name - Jainam Patel
// Student ID - 110123663

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <time.h>
#include <ftw.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 8080
#define ALLOWED_CLIENTS 100
#define ALLOWED_MESSAGE_SIZE 1024

const char *targetFilename; // The filename to search for

void pclientrequest(int clientSocket);
int perfromSearchAndGetInfo(int clientSocket, const char *filename, char *result);
int performSearchAndCompressByCommands(int clientSocket, char commandEnteredByUser[]);
int isResponseSend = 0;

void sigint_handler(int signum)
{
    system("rm config.txt");
    // You can add any cleanup code or additional actions here
    exit(0);
}

int main()
{
    if (signal(SIGINT, sigint_handler) == SIG_ERR)
    {
        fprintf(stderr, "Failed to register SIGINT handler\n");
        return 1;
    }
    int mainServerSocket, clientSocketsArr[ALLOWED_CLIENTS] = {0};
    struct sockaddr_in serverObj, clientObj;
    socklen_t addrSize = sizeof(struct sockaddr_in);

    if ((mainServerSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    int reuse = 1;
    if (setsockopt(mainServerSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1)
    {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

    memset(&serverObj, 0, sizeof(serverObj));
    serverObj.sin_family = AF_INET;
    serverObj.sin_addr.s_addr = INADDR_ANY;
    serverObj.sin_port = htons(PORT);

    if (bind(mainServerSocket, (struct sockaddr *)&serverObj, sizeof(serverObj)) == -1)
    {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    if (listen(mainServerSocket, 100) == -1)
    {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    int connectedClients = 0;

    while (connectedClients < ALLOWED_CLIENTS)
    {
        if ((clientSocketsArr[connectedClients] = accept(mainServerSocket, (struct sockaddr *)&clientObj, &addrSize)) == -1)
        {
            perror("Error accepting connection");
            continue;
        }

        printf("Connection accepted from %s\n", inet_ntoa(clientObj.sin_addr));

        pid_t pid = fork();
        if (pid == -1)
        {
            perror("Error forking process");
            exit(EXIT_FAILURE);
        }
        else if (pid == 0)
        {
            close(mainServerSocket);
            pclientrequest(clientSocketsArr[connectedClients]);
            close(clientSocketsArr[connectedClients]);
            exit(EXIT_SUCCESS);
        }
        else
        {
            close(clientSocketsArr[connectedClients]);
            connectedClients++;
        }
    }

    close(mainServerSocket);

    return 0;
}

void pclientrequest(int clientSocket)
{
    char commandEnteredByUser[ALLOWED_MESSAGE_SIZE];

    while (1)
    {
        ssize_t n = read(clientSocket, commandEnteredByUser, sizeof(commandEnteredByUser));
        if (n <= 0)
        {
            printf("Client disconnected.\n");
            break;
        }
        commandEnteredByUser[n] = '\0';

        printf("Received from client: %.*s", (int)n, commandEnteredByUser);
        quitc if (strncmp(commandEnteredByUser, "", 5) == 0)
        {
            printf("Received 'quitc' command. Exiting...\n");
            break;
        }
        else if (strncmp(commandEnteredByUser, "getfn ", 6) == 0)
        {
            char filename[ALLOWED_MESSAGE_SIZE];
            sscanf(commandEnteredByUser, "getfn %s", filename);
            char result[ALLOWED_MESSAGE_SIZE];

            perfromSearchAndGetInfo(clientSocket, filename, result);
        }
        else if (strncmp(commandEnteredByUser, "getfz", 5) == 0)
        {
            performSearchAndCompressByCommands(clientSocket, commandEnteredByUser);
        }
        else if (strncmp(commandEnteredByUser, "getft", 5) == 0)
        {
            performSearchAndCompressByCommands(clientSocket, commandEnteredByUser);
        }
        else if (strncmp(commandEnteredByUser, "getfdb", 6) == 0)
        {
            performSearchAndCompressByCommands(clientSocket, commandEnteredByUser);
        }
        else if (strncmp(commandEnteredByUser, "getfda", 6) == 0)
        {
            performSearchAndCompressByCommands(clientSocket, commandEnteredByUser);
        }
    }
}

int perfromSearchAndGetInfo(int clientSocket, const char *filename, char *result)
{
    // Use the find command to search for the file
    char findCommandBuffer[ALLOWED_MESSAGE_SIZE];
    snprintf(findCommandBuffer, sizeof(findCommandBuffer), "find ~/UOW -type f -name %s", filename);

    FILE *fp = popen(findCommandBuffer, "r");
    if (fp == NULL)
    {
        perror("popen");
        return 0;
    }

    // Read the first line from the find command output
    char foundFilePath[ALLOWED_MESSAGE_SIZE];

    if (fgets(foundFilePath, sizeof(foundFilePath), fp) != NULL)
    {
        // Remove trailing newline character
        size_t len = strlen(foundFilePath);
        if (len > 0 && foundFilePath[len - 1] == '\n')
        {
            foundFilePath[len - 1] = '\0';
        }

        // File found, get its information
        struct stat fileStat;
        if (stat(foundFilePath, &fileStat) == 0)
        {
            snprintf(result, ALLOWED_MESSAGE_SIZE, "Filename: %s\nSize: %ld bytes\nDate created: %sFile permissions: %o\n",
                     filename, fileStat.st_size, ctime(&fileStat.st_ctime), fileStat.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO));
            pclose(fp);
            write(clientSocket, result, strlen(result));
            return 1;
        }
    }
    else
    {
        // File not found
        printf("File not found\n");

        // Send "File not found" commandEnteredByUser to the client
        char notFoundMessage[] = "File not found";
        size_t messageLength = strlen(notFoundMessage);

        // Send the commandEnteredByUser content
        write(clientSocket, notFoundMessage, messageLength);

        // Close the pipe
        pclose(fp);

        return 0;
    }
}

int performSearchAndCompressByCommands(int clientSocket, char commandEnteredByUser[])
{
    // Use the find command to search for files based on size and create a tar.gz file
    char findCommandBuffer[ALLOWED_MESSAGE_SIZE];
    system("rm -f temp.tar.gz");
    if (strncmp(commandEnteredByUser, "getfz", 5) == 0)
    {
        long size1, size2;
        sscanf(commandEnteredByUser, "getfz %ld %ld", &size1, &size2);
        printf("====> size1: %ld, size2: %ld\n", size1, size2);

        snprintf(findCommandBuffer, sizeof(findCommandBuffer), "find ~/UOW -type f -size +%ldc -size -%ldc -exec tar -rf temp.tar.gz --no-recursion {} + | gzip > temp.tar.gz", size1, size2);
    }
    else if (strncmp(commandEnteredByUser, "getft", 5) == 0)
    {
        char extensions[ALLOWED_MESSAGE_SIZE];
        sscanf(commandEnteredByUser, "getft %[^\n]", extensions);

        int i = 0;
        // Split the extension list and add additional -o conditions
        char *token = strtok(extensions, " ");
        while (token != NULL)
        {
            if (i == 0)
            {
                snprintf(findCommandBuffer, sizeof(findCommandBuffer), "find ~/UOW -type f \\( -name '*.%s' ", token);
            }
            else
            {
                snprintf(findCommandBuffer + strlen(findCommandBuffer), sizeof(findCommandBuffer) - strlen(findCommandBuffer), "-o -name '*.%s' ", token);
            }
            i++;
            token = strtok(NULL, " ");
        }

        snprintf(findCommandBuffer + strlen(findCommandBuffer), sizeof(findCommandBuffer) - strlen(findCommandBuffer), "\\) -exec tar -rf temp.tar.gz {} +");
    }
    else if (strncmp(commandEnteredByUser, "getfdb", 6) == 0)
    {
        char date[ALLOWED_MESSAGE_SIZE];
        sscanf(commandEnteredByUser, "getfdb %[^\n]", date);

        snprintf(findCommandBuffer, sizeof(findCommandBuffer), "find ~/UOW -type f ! -newermt %s -exec tar -rf temp.tar.gz {} +", date);
    }
    else if (strncmp(commandEnteredByUser, "getfda", 6) == 0)
    {
        char date[ALLOWED_MESSAGE_SIZE];
        sscanf(commandEnteredByUser, "getfda %[^\n]", date);

        snprintf(findCommandBuffer, sizeof(findCommandBuffer), "find ~/UOW -type f -newermt %s -exec tar -rf temp.tar.gz {} +", date);
    }

    printf("=====> findCommandBuffer : %s\n", findCommandBuffer);
    FILE *fp = popen(findCommandBuffer, "r");
    if (fp == NULL)
    {
        perror("popen");
        return 0;
    }

    // Close the pipe
    pclose(fp);

    // Wait for the tar.gz file to be fully generated
    sleep(2); // You may need to adjust the sleep duration based on the size of your files

    struct stat fileStat;
    if (stat("temp.tar.gz", &fileStat) == 0)
    {
        if (fileStat.st_size > 0)
        {
            write(clientSocket, "File has been generated and the name is : temp.tar.gz", strlen("File has been generated and the name is : temp.tar.gz"));
        }
        else
        {
            printf("File not found\n");

            // Send "File not found" commandEnteredByUser to the client
            char notFoundMessage[] = "File not found";
            size_t messageLength = strlen(notFoundMessage);

            // Send the commandEnteredByUser content
            write(clientSocket, notFoundMessage, messageLength);
        }
    }
    else
    {
        printf("File not found\n");

        // Send "File not found" commandEnteredByUser to the client
        char notFoundMessage[] = "File not found";
        size_t messageLength = strlen(notFoundMessage);

        // Send the commandEnteredByUser content
        write(clientSocket, notFoundMessage, messageLength);
    }

    return 1;
}
