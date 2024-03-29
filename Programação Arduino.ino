#include <Ethernet.h>
#include <SPI.h>

//definindo a porta analogia 0
#define analogPin1 A0

//variaveis para a configuraçao de internet

byte mac[] = {xxxx, xxxx, xxxx, xxxx, xxxx, xxxx};
IPAddress ip(xxx, xxx, xxx, xxx);
IPAddress gateway(xxx, xxx,xxx, xxx);
IPAddress subnet(xxx, xxx, xxx, xxx);
EthernetServer server(xx);

//fim das definiçoes de internet

//variavel para controle das condiçoes do sistema de irrigação
boolean ativo1 = false;
boolean ativo2 = false;
boolean ativo3 = false;
boolean ativo4 = false;

// valor que assumira a leitura do sensor de umidade
int valor_analogico = 0;

// variavel do rele indicando que sera ligado na na porta 9
int RELE1 = 9;

// funçao carregada ao iniciar o Arduino, para setar as variaveis
void setup()
{
  //Inicia a conexao com a rede local e inicia o servidor web
  Ethernet.begin(mac, ip, gateway, subnet);
  server.begin();

  //colocando a o pino 9 como saida
  pinMode(RELE1, OUTPUT);
  //O rele utilizado recebe o valor 0 para ativar e nao o valor 1 , entao colocando no HIGH garante que ele esteja desligado ao iniciar o programa
  digitalWrite(RELE1, HIGH);
  //colocando o pino A0 como entrada
  pinMode(analogPin1, INPUT);

}

//funçao que fica sempre em funcionamento é a que faz o sistema funcionar
void loop()
{
  EthernetClient client = server.available();
  //Se o cliente esta tentando se conectar
  if (client)
  {
    boolean continua = true;
    String linha = "";
    //Enquanto o cliente estiver conectado
    while (client.connected())
    {
      if (client.available()) {
        //Ler toda requisiçao feita pelo cliente e colocar na variavel c
        char c = client.read();
        linha.concat(c);
        // Leu toda a requicição feita pelo cliente
        if (c == '\n' && continua)
        {
          //cabeçalho de rede
          client.println("HTTP/1.1 200 OK");
          // As proximas linhas fazem com que o Arduino aceite requisiçao de qualquer origem, nao sendo necessariamente apenas local
          client.println("Content-Type: text/javascript");
          client.println("Access-Control-Allow-Origin: *");
          client.println();

          // A requisição vem do cliente dessa forma 192.168.0.8/?acao=

          int inicio = linha.indexOf("?");//procura ate a interrogação
          if (inicio > -1) { //verifica se o comando veio
            inicio = inicio + 6; //pega os caracteres depois da barra /
            int fim  = inicio + 3; //depois do sinal igual espera mais 3 caracteres
            String acao = linha.substring(inicio, fim); //recupera o valor do comando

            //Essa linha coloca o valor lido pelo sensor na variavel
            valor_analogico = analogRead(analogPin1);
            //Despois de lido o valor é mapeado de 0 a 100
            int valor = map(valor_analogico, 0, 1023, 0 , 100);

            //As linhas a seguir são referentes aos perfis de irrigação selecionados pelo usuario, valores adotados apos varios testes
            //Açao do perfil 01
            if ( acao == "001" && valor >= 60 && valor <= 100 ) {
              digitalWrite(RELE1, LOW);
              ativo1 = true;
           
            }
            else if ( valor < 60 && ativo1 == true )
            {
              digitalWrite(RELE1, HIGH);
              ativo1 = false;

            }


            // fim da ação

            //Açao do perfil 02
            if ( acao == "002" && valor >= 40 && valor <= 59 ) {
              digitalWrite(RELE1, LOW);
              ativo2 = true;

            }
            else if ( valor < 40 && ativo2 == true )
            {
              digitalWrite(RELE1, HIGH);
              ativo2 = false;

            }

            // fim da ação

            //Açao do perfil 03
            if ( acao == "003" && valor >= 33 && valor <= 39 ) {
              digitalWrite(RELE1, LOW);
              ativo3 = true;

            }

            else if ( valor < 33 && ativo3 == true )
            {
              digitalWrite(RELE1, HIGH);
              ativo3 = false;

            }

            // fim da ação

            //Modo Manual
            if ( acao == "005") {
              digitalWrite(RELE1, LOW);

            }

            else if ( acao == "006") {
              digitalWrite(RELE1, HIGH);
            }
            //Fim do modo Manual

            //Para sistema
            if ( acao == "007") {
              digitalWrite(RELE1, HIGH);

            }

            else {}

            //Aqui é a função dados setada no parte do cliente, onde o mesmo faz requisiçao e é retornado os valores abaixo.
            //Retorna para o cliente o valor do sensor
            client.print("dados({ sensor1 : ");
            client.print(valor);
            client.print(",");
            //Retorna para o cliente o estado do rele ou como foi setado o estado sistema se esta ligado ou desligado
            client.print(" RELE1 : ");
            client.print(digitalRead(RELE1));
            client.print("})");

          }
          break;
        }
        //Se ainda tiver requisiçoes ele volta todo processo
        if (c == '\n') {
          continua = true;
        }
        //Se nao, ele nao faz mais os processos
        else if (c != '\r') {
          continua = false;
        }
      }
    }
    delay(1);
    client.stop();
  }
}
