// Name - Jainam Patel
// Student ID - 110123663

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define MIRRORIP "127.0.0.1"
#define MIRRORPORT 8081
#define MAX_msg_SIZE 1024

int clientCountNo = 0;
char buffer[32];

void readFromServer(int clientSocket);
void writeToServer(int clientSocket);

int main()
{
    int clientSocket;
    struct sockaddr_in serverAddress;
    char msg[MAX_msg_SIZE];

    // Creating socket
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Error creating the socket");
        exit(EXIT_FAILURE);
    }

    // First, Read the value from the configuration file
    int configFile = open("config.txt", O_CREAT | O_RDWR, 0777);
    if (configFile != -1)
    {
        read(configFile, buffer, sizeof(buffer));
        sscanf(buffer, "%d", &clientCountNo);
    }
    else
    {
        // If the configuration file doesn't exist, we will first initialize it with 0
        snprintf(buffer, sizeof(buffer), "%d", clientCountNo);
        write(configFile, buffer, snprintf(buffer, sizeof(buffer), "%d", clientCountNo));
        clientCountNo = 0;
    }
    clientCountNo++;

    memset(&serverAddress, 0, sizeof(serverAddress));
    lseek(configFile, 0, SEEK_SET);
    if (clientCountNo <= 4)
    {
        // For the first 4 clients we will connect to server
        printf("clientCountNo :%d\n", clientCountNo);

        // Setting up the server address structure
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(PORT);
        inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr);

        if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
        {
            perror("Error while connecting to server");
            exit(EXIT_FAILURE);
        }

        printf("Connected to  the server\n");
        snprintf(buffer, sizeof(buffer), "%d", clientCountNo);
        printf("buffer: %s\n", buffer);
        write(configFile, buffer, sizeof(buffer));
    }
    else if (clientCountNo <= 8)
    {
        // Now, connecting client for the 5 to 8 in the mirrors
        close(clientSocket);
        printf("asdsfafs\n");

        // Create a new socket
        if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        {
            perror("Error creating socket");
            exit(EXIT_FAILURE);
        }

        printf("asdsfafs\n");

        // Connect with the mirror
        printf("Connecting to the mirror...\n");
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(MIRRORPORT);
        inet_pton(AF_INET, MIRRORIP, &serverAddress.sin_addr);

        // Connect with the server or mirror
        if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
        {
            perror("Error connecting to target");
            exit(EXIT_FAILURE);
        }
        printf("Connected to mirror\n");
        snprintf(buffer, sizeof(buffer), "%d", clientCountNo);
        printf("buffer: %s\n", buffer);
        write(configFile, buffer, sizeof(buffer));
    }
    else
    {
        // Alternate between server and mirror for additional connections
        if (clientCountNo % 2 == 1)
        {
            // Connect to the server for odd-numbered clients
            // Setup server address struct
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(PORT);
            inet_pton(AF_INET, SERVER_IP, &serverAddress.sin_addr);

            if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
            {
                perror("Error connecting to server");
                exit(EXIT_FAILURE);
            }

            printf("Connected to server\n");
            snprintf(buffer, sizeof(buffer), "%d", clientCountNo);
            printf("buffer: %s\n", buffer);
            write(configFile, buffer, sizeof(buffer));
        }
        else
        {
            // Connect with the mirror for even-numbered clients
            close(clientSocket);
            // Create a new socket
            if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
            {
                perror("Error creating socket");
                exit(EXIT_FAILURE);
            }

            printf("asdsfafs\n");
            // Connect to the mirror
            printf("Connecting to the mirror...\n");
            serverAddress.sin_family = AF_INET;
            serverAddress.sin_port = htons(MIRRORPORT);
            inet_pton(AF_INET, MIRRORIP, &serverAddress.sin_addr);
            // Connect to the target (server or mirror)
            if (connect(clientSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
            {
                perror("Error connecting to target");
                exit(EXIT_FAILURE);
            }
            printf("Connected to mirror\n");
            snprintf(buffer, sizeof(buffer), "%d", clientCountNo);
            printf("buffer: %s\n", buffer);
            write(configFile, buffer, sizeof(buffer));
        }
    }

    while (1)
    {
        // Write to the server
        writeToServer(clientSocket);

        // Read from the server
        readFromServer(clientSocket);
    }

    // Close client socket
    close(clientSocket);

    return 0;
}

// Function to read data from the server
void readFromServer(int clientSocket)
{
    char msg[MAX_msg_SIZE];
    printf("Waiting for server response\n");

    // Receive a response from the server
    ssize_t bytesRead = read(clientSocket, msg, sizeof(msg));
    if (bytesRead <= 0)
    {
        // Server disconnected or error
        printf("Server disconnected.\n");
        exit(EXIT_FAILURE);
    }

    // Check if the received msg contains the filename "temp.tar.gz"
    if (strstr(msg, "temp.tar.gz") != NULL)
    {
        printf("f23project\n");
        // Create a directory and copy the file to it
        system("mkdir ~/f23project");
        system("cp temp.tar.gz ~/f23project");
    }

    // Print received msg
    printf("Received from server: %.*s\n", (int)bytesRead, msg);
}

// Function to write data to the server
void writeToServer(int clientSocket)
{
    char msg[MAX_msg_SIZE];

    // Send a msg to the server
    printf("Client$ ");
    fgets(msg, sizeof(msg), stdin);

    // Remove the newline character if present
    size_t length = strlen(msg);
    if (length > 0 && msg[length - 1] == '\n')
    {
        msg[length - 1] = '\0';
    }

    // Now, writing the msg to the server
    write(clientSocket, msg, strlen(msg));

    // Check first if the client wants to exit or not
    if (strcmp(msg, "exit") == 0)
    {
        printf("Exiting... !!!\n");

        exit(EXIT_SUCCESS);
    }
}