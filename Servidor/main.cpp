#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string>
#include <fstream>

using namespace std;

void leerArchivoUsuarios(/*char user[1024],char password[1024]*/char RecvBuff[1024]);

class Servidor{
public:
    WSADATA wsaData; //variable para inicializar la libreria winsock2
    //socket de conexion y socket de comunicacion con el cliente
    SOCKET conexion_socket,comunicacion_socket;
    struct sockaddr_in servidor;//direccion del socket servidor
    struct sockaddr_in cliente;//direccion del socket cliente
    struct hostent *hp;
    int resp,stsize;
    char SendBuff[1024],RecvBuff[1024],user[1024],password[1024];//enviar y recibir mensajes

    Servidor(){

       //Inicializamos la libreria winsock2
       cout<<"Inicializando Winsock..."<<endl;
       resp=WSAStartup(MAKEWORD(1,0),&wsaData);
       if(resp){
        cout<<"Error al inicializar socket"<<WSAGetLastError()<<endl;
        getchar();
       }
       cout<<"Winsock Inicializado"<<endl;

       //Obtenemos la IP que usar� nuestro servidor...
       // en este caso localhost indica nuestra propia m�quina...
       hp=(struct hostent *)gethostbyname("localhost");
       if(!hp){
        cout<<"No se ha encontrado servidor..."<<endl;
        getchar();
        WSACleanup();
       }

       // Creamos el socket...
       conexion_socket=socket(AF_INET,SOCK_STREAM, 0);
       if(conexion_socket==INVALID_SOCKET){
        cout<<"Error al crear socket"<<WSAGetLastError()<<endl;
        getchar();
        WSACleanup();
       }
       cout<<"Socket creado."<<endl;

       //asociamos la address al socket
       memset(&servidor, 0, sizeof(servidor)) ;
       memcpy(&servidor.sin_addr, hp->h_addr, hp->h_length);
       servidor.sin_family = hp->h_addrtype;
       servidor.sin_port = htons(6000);

       // Asociamos ip y puerto al socket
       resp=bind(conexion_socket, (struct sockaddr *)&servidor, sizeof(servidor));
       if(resp==SOCKET_ERROR){
        cout<<"Error al asociar puerto e ip al socket"<<WSAGetLastError()<<endl;
        closesocket(conexion_socket);
        WSACleanup();
        getchar();
       }
       cout<<"Bind Success"<<endl;

       //ponemos a escuchar al servidor
       if(listen(conexion_socket, 1)==SOCKET_ERROR){
        cout<<"Error al habilitar conexiones entrantes"<<WSAGetLastError()<<endl;
        closesocket(conexion_socket);
        WSACleanup();
        getchar();
       }

       // Aceptamos conexiones entrantes
       cout<<"Esperando conexiones entrantes..."<<endl;
       stsize=sizeof(struct sockaddr);
       comunicacion_socket=accept(conexion_socket,(struct sockaddr *)&cliente,&stsize);
       if(comunicacion_socket==INVALID_SOCKET){
          cout<<"Error al aceptar conexi�n entrante "<<WSAGetLastError()<<endl;
          closesocket(conexion_socket);
          WSACleanup();
          getchar();
       }
       cout<<"Conexion entrante desde: "<<inet_ntoa(cliente.sin_addr)<<endl;

       // Como no vamos a aceptar m�s conexiones cerramos el socket escucha
       //closesocket(conexion_socket);
    }
    //metodo comun para recibir mensajes
    void recibir(){
        recv (comunicacion_socket, RecvBuff, sizeof(RecvBuff), 0);
        cout<<"El cliente dice: "<<RecvBuff<<endl;
        log(RecvBuff);
        memset(RecvBuff,0,sizeof(RecvBuff));
    }
    //metodo que recibe el usuario y la contrase�a
    void recibirUserPassword(){
        /*recv (comunicacion_socket, user, sizeof(user), 0);
        cout<<"Usuario: "<<user<<endl;
        recv (comunicacion_socket, password, sizeof(password), 0);
        cout<<"Contrase�a: "<<password<<endl;
        //leo archivo y verifico que sea igual a una contrase�a
        leerArchivoUsuarios(user,password);
        memset(user,0,sizeof(user));
        memset(password,0,sizeof(password));*/
        recv (comunicacion_socket, RecvBuff, sizeof(RecvBuff), 0);
        cout<<"El cliente dice: "<<RecvBuff<<endl;
        //log(RecvBuff);
        //leo archivo y verifico que sea igual a una contrase�a
        leerArchivoUsuarios(RecvBuff);
        memset(RecvBuff,0,sizeof(RecvBuff));
    }
    //metodod comun para enviar mensajes
    void enviar(){
        cout<<"Escribe el mensaje a enviar: ";
        cin>>this->SendBuff;
        log(SendBuff);
        send(comunicacion_socket, SendBuff, sizeof(SendBuff), 0);
        memset(SendBuff, 0, sizeof(SendBuff));
        cout << "Mensaje enviado!" <<endl;
    }

    void cerrar(){
        // Cerramos el socket de la conexion
        closesocket(conexion_socket);
        WSACleanup();
        cout<<"Socket cerrado"<<endl;
        //log.close();
    }

    void log(string msg){
        // Declaramos las variables
        ofstream log;
        string log_file;

        // Creamos el archivo de log
        log_file.assign("server.log");
        log.open(log_file.c_str());

        // Escribimos una l�nea con el nombre del archivo
        log << "2020-10-15: " << msg << std::endl;

        // Escribimos en el log
        //log << "Esta es una linea del log" << std::endl;

        // Cerramos el archivo
        //log.close();
    }


};

int main(int argc, char *argv[])
{
    Servidor *server = new Servidor();

    while(true){
        server->recibirUserPassword();
        server->enviar();
    }
    server->cerrar();
    return 0;
}
//funcion para leer el archivo
void leerArchivoUsuarios(/*char user[1024],char password[1024]*/char RecvBuff[1024]){
   ifstream usuarios;
   string linea,linea2;
   std::string usuario(RecvBuff); //convierto el char a string
   //std::string contrase�a(password);
   int encontrado = 0;
   usuarios.open("usuarios.txt",ios::in); // abro el archivo en modo lectura
   if(usuarios.fail()){
    cout<<"No se pudo abrir el archivo"<<endl;
   }
   while(getline(usuarios,linea)){
        if(linea.find(usuario) != string::npos){
            cout<<linea<<endl;
            encontrado = 1;
        }
   }
   if(encontrado == 0){
    cout<<"No se encontro el usuario "<<usuario<<endl;
   }
   usuarios.close();

}
