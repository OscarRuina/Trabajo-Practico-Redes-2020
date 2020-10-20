#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <string>
#include <fstream>
#include <ctime>
#include <sstream>

using namespace std;


int countLog;
int Intentos;

int leerArchivoUsuarios(char RecvBuff[1024]);
void log(string msg);

class Servidor{
public:
    WSADATA wsaData; //variable para inicializar la libreria winsock2
    //socket de conexion y socket de comunicacion con el cliente
    SOCKET conexion_socket,comunicacion_socket;
    struct sockaddr_in servidor;//direccion del socket servidor
    struct sockaddr_in cliente;//direccion del socket cliente
    struct hostent *hp;
    int resp,stsize;
    char SendBuff[1024],RecvBuff[1024];//enviar y recibir mensajes
    int RespLogin;

    Servidor(){
        countLog = 1;
        RespLogin = 0;
        log("INICIA SERVIDOR");
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


    }   //FIN SERVIDOR



    //metodo comun para recibir mensajes
    void recibir(){
        recv (comunicacion_socket, RecvBuff, sizeof(RecvBuff), 0);
        cout<<"El cliente dice: "<<RecvBuff<<endl;
        log(RecvBuff);
        memset(RecvBuff,0,sizeof(RecvBuff));
    }
    //metodo que recibe el usuario y la contrase�a
    void recibirUserPassword(){
        recv (comunicacion_socket, RecvBuff, sizeof(RecvBuff), 0);
        if ( RecvBuff[0] != '\0' ){
            //cout<<"Usuario y Contrase�a: "<<RecvBuff<<endl;
            if (leerArchivoUsuarios(RecvBuff) == 0 ){ //1-valida usuario y pass
                Intentos = Intentos + 1;      //2-intentos
            }
            if ( Intentos == 3 ){
                log("Usuario ingreso tres veces mal la contrasena");
                enviarCierre();                                   //explota
            }
        }
        memset(RecvBuff,0,sizeof(RecvBuff));
    }

    void enviarCierre(){
        SendBuff[0] = '4';
        send(comunicacion_socket, SendBuff, sizeof(SendBuff), 0);
        memset(SendBuff, 0, sizeof(SendBuff));
    }
    //metodo comun para enviar mensajes
    void enviar(){
        cout<<"Escribe el mensaje a enviar: ";
        cin>>this->SendBuff;
        log(SendBuff);
        send(comunicacion_socket, SendBuff, sizeof(SendBuff), 0);
        memset(SendBuff, 0, sizeof(SendBuff));
        cout << "Mensaje enviado!" <<endl;
    }

    void enviarIntento(){
        send(comunicacion_socket, SendBuff, sizeof(SendBuff), 0);
        memset(SendBuff, 0, sizeof(SendBuff));
    }

    void cerrarConexion(){
        // Cerramos el socket de la conexion
        closesocket(conexion_socket);
        WSACleanup();
        cout<<"Socket cerrado"<<endl;
    }

    void cerrarComunicacion(){
        // Cerramos el socket de la comunicacion
        closesocket(comunicacion_socket);
        WSACleanup();
        cout<<"Socket cerrado"<<endl;
    }

};

int main(int argc, char *argv[])
{
    Servidor *server = new Servidor();
    Intentos = 0;

    while(true){
        server->recibirUserPassword();
        server->enviarIntento();
    }

    server->cerrarConexion();
    return 0;
}

//Funcion Log
void log(string msg){
        // Declaramos las variables
        ofstream log;
        string log_file;

        //Abrir el archivo
        log_file.assign("server.log");
        log.open(log_file.c_str(),ios::app);

        //Declaramos la fecha/hora del dia.
        time_t now = time(0);
        tm* time = localtime(&now);
        int dia = time-> tm_mday;
        int mes = time-> tm_mon + 1;
        int anio = 1900 + time-> tm_year;
        int hora = time-> tm_hour;
        int minu = time-> tm_min;

        // Escribimos inicio del log
        if (countLog == 1) {
            log << anio <<"-"<< mes <<"-"<< dia <<"_"<< hora <<":"<< minu << ": ==================" << std::endl;
            log << anio <<"-"<< mes <<"-"<< dia <<"_"<< hora <<":"<< minu << ": " << msg << std::endl;
            log << anio <<"-"<< mes <<"-"<< dia <<"_"<< hora <<":"<< minu << ": ==================" << std::endl;
            countLog += 1;
        }else{
            // Escribimos en el log
            log << anio <<"-"<< mes <<"-"<< dia <<"_"<< hora <<":"<< minu << ": " << msg << std::endl;
        }

        // Creamos el archivo de log
        //log_file.assign("server.log");
        //log.open(log_file.c_str());

        // Escribimos una l�nea con el nombre del archivo
        //log << "2020-10-15: " << msg << std::endl;

        // Escribimos en el log
        //log << "Esta es una linea del log" << std::endl;


        // Cerramos el archivo
        log.close();
    }

//funcion para leer el archivo
int leerArchivoUsuarios(char RecvBuff[1024]){
   ifstream usuarios;
   string linea,linea2;
   std::string usuario(RecvBuff); //convierto el char a string
   int encontrado  = 0;
   usuarios.open("usuarios.txt",ios::in); // abro el archivo en modo lectura
   if(usuarios.fail()){
    cout<<"No se pudo abrir el archivo"<<endl;
    log("No se pudo abrir el archivo");
   }
   while(getline(usuarios,linea)){
        if(linea.find(usuario) != string::npos){
            cout<<linea<<endl;                         //aca tendria el usuario y pass encontrado.
            log("El Usuario: " + linea + " se logeo");
            encontrado = 1;
        }
   }
   if(encontrado == 0){
    cout<<"Usuario o contrasena no encontrado: "<<usuario<<endl;
    log("Usuario o contrasena no encontrado");
   }
   usuarios.close();
   return encontrado;
}
