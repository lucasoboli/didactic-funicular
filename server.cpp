/*
    C socket server example
*/

#include <arpa/inet.h> //inet_addr
#include <pthread.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h> //strlen
#include <sys/socket.h>
#include <unistd.h> //write

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <thread>
#include <vector>

void *myThreadFun(void *vargp) {
  sleep(1);
  printf("Printing GeeksQuiz from Thread \n");
  return NULL;
}

using namespace std;

void worker() {
  while (1) {
    cout << "te thread" << endl;
    sleep(1);
  }
}

int main(int argc, char *argv[]) {
  vector<thread> workers;
  map<string, vector<string>> userMap;

  int socket_desc, client_sock, c;
  struct sockaddr_in server;

  string serverPort = argv[1];

  // Create socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);
  if (socket_desc == -1) {
    printf("Could not create socket");
  }
  puts("Socket created");

  // Prepare the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(stoi(serverPort.c_str()));

  // Bind
  if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0) {
    // print the error message
    perror("bind failed. Error");
    return 1;
  }
  puts("bind done");

  // Listen
  listen(socket_desc, 3);

  // Accept and incoming connection
  puts("Waiting for incoming connections...");
  c = sizeof(struct sockaddr_in);

  // accept connection from an incoming client
  struct sockaddr_in client;

  while (1) {
    // Aceita uma conexão socket e cria uma thread com seu identificador
    client_sock =
        accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);

    workers.push_back(thread([&userMap, client_sock, client, socket_desc]() {
      int readSize;
      char clientName[20];
      char bufferC[1024];
      char bufferA[1024];
      string clientCommand, clientArg;

      if (client_sock < 0) {
        perror("accept failed");
        return 1;
      }

      puts("Connection accepted");

      // Lê o nome do cliente do socket
      recv(client_sock, clientName, 20, 0);
      cout << "Conectado cliente: " << clientName << endl;

      while (1) {
        readSize = recv(client_sock, bufferC, 1024, 0);

        // Checa se for uma desconexão. Se for dá flush e break
        if (readSize == 0) {
          cout << "Cliente " << clientName << " desconectado!" << endl;
          fflush(stdout);
          break;
        } else if (readSize == -1) {
          perror("recv failed");
        }

        // Se não pega e trata o comando recebido
        clientCommand = bufferC;
        cout << "(" << clientName << ") COMANDO RECEBIDO: " << clientCommand
             << endl;

        if (clientCommand.compare("echo") == 0) {
          // Lê o argumento do echo
          recv(client_sock, bufferA, 1024, 0);
          clientArg = bufferA;
          write(client_sock, clientArg.c_str(), clientArg.size() + 1);

          memset(bufferC, 0, 1024);
          memset(bufferA, 0, 1024);
        } else if (clientCommand.compare("send") == 0) {
          string nomeDoArquivo;
          ofstream myfile;

          recv(client_sock, bufferA, 1024, 0);
          nomeDoArquivo = bufferA;
          nomeDoArquivo = "SERV" + nomeDoArquivo;

          myfile.open(nomeDoArquivo);

          memset(bufferA, 0, 1024);

          while (1) {
            recv(client_sock, bufferA, 1024, 0);

            clientArg = bufferA;
            cout << clientArg << endl;

            if (clientArg.compare("!EOF!") == 0) {
              memset(bufferA, 0, 1024);
              break;
            }

            myfile << clientArg << endl; // Recebe a linha

            write(client_sock, "ACK\0", 4); // Manda um ACK
            memset(bufferA, 0, 1024);
          }

          myfile.close();

          string done = "Arquivo inserido com sucesso";

          userMap[clientName].push_back(nomeDoArquivo.substr(4));

          for (float i = 0; i < 100000; i++)
            ; // delay para mandar o fim do arquivo

          write(client_sock, done.c_str(), done.size() + 1);

          memset(bufferC, 0, 1024);
          memset(bufferA, 0, 1024);
        } else if (clientCommand.compare("list") == 0) {
          write(client_sock, "============", 15); // Header da lista
          recv(client_sock, bufferA, 1024,
               0); // Espera o ACK do cliente que recebeu a linha

          for (auto nomeDoArquivo : userMap[clientName]) {
            for (float i = 0; i < 100000; i++)
              ; // delay para mandar cada linha da lista

            nomeDoArquivo = "* " + nomeDoArquivo;
            write(client_sock, nomeDoArquivo.c_str(), nomeDoArquivo.size() + 1);
            recv(client_sock, bufferA, 1024,
                 0); // Espera o ACK do cliente que recebeu a linha
          }
          write(client_sock, "============", 15); // Bunda da lista
          recv(client_sock, bufferA, 1024,
               0); // Espera o ACK do cliente que recebeu a linha
          string endOF = "!EOF!";

          for (float i = 0; i < 100000; i++)
            ; // delay para mandar o fim do arquivo

          write(client_sock, endOF.c_str(), endOF.size() + 1);

          string done = "Mostrar lista concluído!";
          for (float i = 0; i < 100000; i++)
            ; // delay para
              // mandar o fim do
              // arquivo

          write(client_sock, done.c_str(), done.size() + 1);
        } else if (clientCommand.compare("get") == 0) {
          string endOF = "!EOF!";
          string nomeDoArquivo;

          recv(client_sock, bufferA, 1024, 0);
          nomeDoArquivo = bufferA;
          nomeDoArquivo = "SERV" + nomeDoArquivo;

          cout << "NOME DO ARQUIVO: " << nomeDoArquivo << endl;

          ifstream myfile(nomeDoArquivo);

          memset(bufferA, 0, 1024);
          sleep(1); // Delay pro cliente ficar pronto pra esperar as mensagens

          string line;
          if (myfile.is_open()) {
            while (getline(myfile, line)) {
              // cout << "ARQV: " << line << endl;

              line += '\0';
              send(client_sock, line.c_str(), line.size() + 1,
                   0); // Envia uma linha
              recv(client_sock, bufferA, 1024,
                   0); // Espera o ACK do server que recebeu a linha
              memset(bufferA, 0, 1024);
            }
            myfile.close();

            for (float i = 0; i < 500000; i++)
              ; // delay para mandar o fim do arquivo

            send(client_sock, endOF.c_str(), endOF.size() + 1, 0);

            string done = "Arquivo enviado com sucesso!";
            for (float i = 0; i < 100000; i++)
              ; // delay para
                // mandar o fim do
                // arquivo

            write(client_sock, done.c_str(), done.size() + 1);
          }
        }
      }
    }));
  }

  std::for_each(workers.begin(), workers.end(),
                [](std::thread &t) { t.join(); });

  return 0;
}