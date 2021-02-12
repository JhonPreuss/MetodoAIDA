#ifndef LEITURA_ARQUIVO_H_INCLUDED
#define LEITURA_ARQUIVO_H_INCLUDED
#include <string>
#include <stdio.h>

//funçao para leitura dos arquivos com as métricas coletadas do tráfego de rede
//essa função irá contar quantas linhas/registros existem nos arquivos que estão sendo processados para AQR
int quantidade_linhas (void){
  FILE *arquivo = fopen("tcpstat_output", "r");
  int x;
  int caractere, existe_linhas = 0;
  int quant_linhas = 0;
    // while para ler o arquivo ate o delimitador"EOF" que indica o  fim de arquivo e contar o numero de linhas
    while((caractere = fgetc(arquivo)) != EOF){
      existe_linhas = 1; // verifica se o arquivo contem dados

    if(caractere == '\n'){
      // incrementa a quantidade de linhas
      quant_linhas++;
    }

   }

    if(existe_linhas){
     //determina a quantidade de loop;
     quant_linhas++;
     x=((quant_linhas)/60)+1;
    }

    return x;
}

int manipula_arquivos(void){
//funçao para manipular e criar arquivos que irão ser/gerar as series temporais
    int flag;
    //Extrai as 60 primeiras linhas de tcpstat_output e copia para tcpstat_output1;
    system("head --lines=60  tcpstat_output> tcpstat_output1");
    //exclui 60 primeiras linhas de tcpstat_output e copia o restante para tcpstat_output2;
    system("sed '1,60d'  tcpstat_output> tcpstat_output2");
    //copia tcpstat_output2 para tcpstat_output;
    system("cp 'tcpstat_output2'  'tcpstat_output'");
    //Extrai as 60 primeiras linhas de tcpstat_output2 e copia para tcpstat_output3;
    system("head --lines=60 tcpstat_output2 > tcpstat_output3");
    flag = 1 ;//1==ok
return flag;
}
//essas funções são chamadas após a analise final dos conjuntos de testes, indicando se naquela serie temporal analisa é normal ou anormal as medidas de AQR
void salva_anomalia(std::string saida){
  //int flag;
   std::string flag;
     FILE *arq_saida;
     arq_saida = fopen(saida.c_str(),"a+");
     fprintf(arq_saida,"anomalia\n");
    flag="anomalia";
     //return flag;**talvez por um cout aqui
}
void salva_normalidade(std::string saida){
  //int flag;
   std::string flag;
     FILE *arq_saida;
     arq_saida = fopen(saida.c_str(),"a+");
     fprintf(arq_saida,"normal\n");
     flag="normalidade";
     //return flag;
}
#endif // LEITURA_ARQUIVO_H_INCLUDED

//funções para criar arquivos de saida que indicam a anomalia ou normalidade em cada um dos 7 conjunto de testes
