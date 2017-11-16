import g4p_controls.*;
import processing.net.*;

Server myServer;
String data;

int toSend;

void setup()
{
   size(520,360);   
   myServer = new Server(this,22522);
}

void draw()
{
    background(255);
    if(keyPressed)
    {
       switch(key)
       {
          case 'z':
          toSend = 1;
          break;
          
          case 'q':
          toSend = 2;
          break;
          
          case 's':
          toSend = 3;
          break;
          
          case 'd':
          toSend = 4;
          break;
          
          case ' ':
          toSend=5;
          break;
       }
    }
    myServer.write(toSend);
    
    toSend = 0;
}