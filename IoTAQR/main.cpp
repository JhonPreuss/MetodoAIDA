/*
Esse é um código em C++ adaptado do código desenvolvido pelo senhor Marcelo A. Righi;
Requisitos:
Bibliotecas:
-CRQA
 - Pacote que possíbilita a ecxecução de calculos e medidas da recorrência
 RInside
 -Fornece Classes C ++ para incorporar códigos e ações  R em aplicativos C ++ (e C).
 akmeans: Adaptive Kmeans algorithm based on threshold
-Pacote R que fornece o algoritmo akmeans para clusterização adaptativa

Definição dos seguintes parâmetros para AQR:
- delay, radius, rescale, normalize, mindiagline, minvertline, tw, whiteline, recpt, side, checkl
- delay e radius são os rincipais parâmetros que devem ser calculados
- radius é um parâmetro que deve ser calculado individualmente para cada uma das medidas da recorência que será calculada

Definição dos seguintes parâmetros para clusterização adpatativa:
- Defina min.k e max.k, min.k = minimo de cluster para iniciar
- d.metric = 1: use métrica de distância euclidiana, caso contrário use métrica de distância de cosseno
- ths4=limiar para decidir se deseja aumentar k ou não


Funcionamento em Etapas:
1 - Forma a primeira série temporal, de acordo com  número de resgistros no arquivo de saída da caputra de tráfego de rede, nessa versão, são analisados a cada 60 linhas
2 - Forma a segunda série temporal, de acordo com  número de resgistros no arquivo de saída da caputra de tráfego de rede, nessa versão, são as 60 linhas subsequentes
3 - Cria o ambiente R dentro da execução C++
4 - Transforma as séries em matriz R para manipulação
5 - Calcula-se as MQRS das duas séries analisadas
5-  Salva o resultado das MQR's em uma matriz
6-  Inicia o pacote Akmeans, e passa como parâmetro a matriz contendo os calulos das MQRs obtidas 
7-  Compara os valores centroids obtidos com os valores de centroids coletados a partir de um conjunto de treinamento.
8-  ao final testa se a grande maioria das comparações obtiver um valor inferior, será indicado anomalia na serie analisada.

*/
#include<stdio.h>
#include<stdlib.h>
#include <RInside.h>
#include<iostream>
#include<fstream>
#include <string>
#define LINHA 7
#define COLUNA 30
#define LINHA2 7
#define COLUNA2 30

//arquivo para funções de manipulaçoes de arquivos
#include "leitura_arquivo.h"
using namespace std;

int main(int argc, char *argv[])
{
    
    //Instancia o ambiente e funçoes R;
    RInside R(argc, argv);
    //variaveis auxiliares
    int s, i, cont, x, aux, n_linhas;
    aux=0;
    std::string nome_matriz, input, par;
    par ="sep =' ', dec = '.'";
    int quantidade_linhas();
    n_linhas = quantidade_linhas();

    do{
        //-----------------------------------------------
        cout <<" Contine...=  "<<aux<< std::endl;
        if(aux>=1390){
        cout <<" Contine...=  "<< std::endl;
        input = fgetc(stdin);
        }
        ///------------------------------------------------
     //executa enquanto aux < n_linhas;
        aux=aux+1;//acrescenta1 ao contador aux;
        int manipulacao;
        manipulacao = manipula_arquivos();
        if(manipulacao == 1){
            //matriz salva em txt
            nome_matriz="X" + std::to_string(aux);
            //tcpstat_output* são os arquivos de saída do módulo de coleta
            //le arquivo tcpstat_output1 via R e denomina de "X"(criando a 1ª Série Temporal);
            //cada linha uma série temporal para cada um dos 7 atributos
            std::string cmd = "X=scan(file='tcpstat_output1');"
                             // escrever uma matriz num arquivo 
                              "matriz <-matrix(X, 8, 60);" //transforma "X" em uma matriz no R;
                              "file.create('"+nome_matriz +"');"
                              "write.table(matriz, file='"+ nome_matriz +"', append = FALSE,"+par+", row.names = TRUE, col.names = TRUE);"
                              //"print(matriz);"
                              "k1 <-c(matriz[2,1:60]);"//separa cada linha com todos os valores para S1;
                              "print(k1);"
                              "k2 <-c(matriz[3,1:60]);"//separa cada linha com todos os valores para S2;
                              "k3 <-c(matriz[4,1:60]);"//separa cada linha com todos os valores para S3;
                              "k4 <-c(matriz[5,1:60]);"//separa cada linha com todos os valores para S4;
                              "k5 <-c(matriz[6,1:60]);"//separa cada linha com todos os valores para S5;
                              "k6 <-c(matriz[7,1:60]);"//separa cada linha com todos os valores para S6;
                              "k7 <-c(matriz[8,1:60]);";//separa cada linha com todos os valores para S7;
            R.parseEval(cmd);

//-------------------------------------------PREPAROU A PRIMEIRA SERIE TEMPORAL AQUI------------------------------//
            //le arquivo tcpstat_output3 pelo R e denomina de "X"(2ª Série temporal);
            nome_matriz="Y" + std::to_string(aux);
            std::string cmd1 ="Y=scan(file='tcpstat_output3');"
                              "matriz2 <-matrix(Y, 8, 60);"//transforma "X" em uma matriz2 no R;
                              "file.create('"+nome_matriz +"');"
                              "write.table(matriz2, file='"+ nome_matriz +"', append = FALSE,"+par+", row.names = TRUE, col.names = TRUE);"
                              "k8 <-c(matriz2[2,1:60]);"//separa cada linha com todos os valores para S1;
                              "k9 <-c(matriz2[3,1:60]);"//separa cada linha com todos os valores para S2;
                              "k10 <-c(matriz2[4,1:60]);"//separa cada linha com todos os valores para S3;
                              "k11 <-c(matriz2[5,1:60]);"//separa cada linha com todos os valores para S4;
                              "k12 <-c(matriz2[6,1:60]);"//separa cada linha com todos os valores para S5;
                              "k13 <-c(matriz2[7,1:60]);"//separa cada linha com todos os valores para S6;
                              "k14 <-c(matriz2[8,1:60]);";//separa cada linha com todos os valores para S7;
            R.parseEval(cmd1);
        //-------------------------------------------PREPAROU A SEGUNDA SERIE TEMPORAL AQUI------------------------------//
        }else{
            cout << "Erro ao manipular os arquivos base para gerar as series temporais";
        }


        //------------------carrega pacote de análise da recorrência no R------------------------------------------------//

        std::string cmd4 ="require(crqa);"


        //------------------abaixo inicializa parametros do pacote CRQA;---------------------------------------------------
                          "delay = 1; rescale = 2; normalize = 0; mindiagline = 2; minvertline = 2;tw = 0; whiteline =FALSE; recpt = FALSE; side = 'both'; checkl = list(do = FALSE, thrshd = 3, datatype = 'numerical',pad = TRUE);"
                          "rqa1 = crqa(k1, k8, delay, embed=1, rescale, radius=5.5, normalize, mindiagline, minvertline, tw, whiteline, recpt, side, checkl);"

                          //AQUI COMEÇA OS CALCULOS//
        //calcula RR(razão da recorrência),DET(determinismo) e ENTR(entropia de Shennon) para S2 TW LP entre a 1ª e 2ª séries temporais;
                          "rqa2 = crqa(k2, k9, delay, embed=1, rescale, radius=5, normalize, mindiagline, minvertline, tw, whiteline, recpt, side, checkl);"

        //calcula RR(razão da recorrência),DET(determinismo) e ENTR(entropia de Shennon) para S3 TW LP entre a 1ª e 2ª séries temporais;
                          "rqa3 = crqa(k3, k10, delay, embed=7, rescale, radius=15, normalize, mindiagline, minvertline, tw, whiteline, recpt, side, checkl);"
        //calcula RR(razão da recorrência),DET(determinismo) e ENTR(entropia de Shennon) para S4 TW LP entre a 1ª e 2ª séries temporais;

                          "rqa4 = crqa(k4, k11, delay, embed=2, rescale, radius=10, normalize, mindiagline, minvertline, tw, whiteline, recpt, side, checkl);"

        //calcula RR(razão da recorrência),DET(determinismo) e ENTR(entropia de Shennon) para S5 TW LP entre a 1ª e 2ª séries temporais;
                          "rqa5 = crqa(k5, k12, delay, embed=6, rescale, radius=6, normalize, mindiagline, minvertline, tw, whiteline, recpt, side, checkl);"

        //calcula RR(razão da recorrência),DET(determinismo) e ENTR(entropia de Shennon) para S6 TW LP entre a 1ª e 2ª séries temporais;
                          "rqa6 = crqa(k6, k13, delay, embed=2, rescale, radius=4, normalize, mindiagline, minvertline, tw, whiteline, recpt, side, checkl);"

        //calcula RR(razão da recorrência),DET(determinismo) e ENTR(entropia de Shennon) para S7 TW LP entre a 1ª e 2ª séries temporais;
                          "rqa7 = crqa(k7, k14, delay, embed=1, rescale, radius=5, normalize, mindiagline, minvertline, tw, whiteline, recpt, side, checkl);"

        //RBIND- funçao para combinar, adicionar linhas em uma matriz de dados matriz
        //as matrize sao preencidas pelas colunas, para manipular cada linha, usa-se a funçao byrow e parametro true byrow=TRUE

                          "MatrizRR<-matrix((rbind(rqa1$RR,rqa2$RR,rqa3$RR,rqa4$RR,rqa5$RR,rqa6$RR,rqa7$RR )),7,1, byrow=T);"
                          "MatrizDET<-matrix((rbind(rqa1$DET,rqa2$DET,rqa3$DET,rqa4$DET,rqa5$DET,rqa6$DET,rqa7$DET )),7,1, byrow=T);"
                          "MatrizMaxL<-matrix((rbind(rqa1$maxL,rqa2$maxL,rqa3$maxL,rqa5$maxL,rqa6$maxL, rqa7$maxL)),6,1, byrow=T);"
                          "MatrizL<-matrix((rbind(rqa3$L)),1,1, byrow=T);"
                          "MatrizENTR<-matrix((rbind(rqa1$ENTR,rqa2$ENTR,rqa3$ENTR,rqa4$ENTR,rqa5$ENTR,rqa6$ENTR,rqa7$ENTR )),7,1, byrow=T);"
                          "MatrizLAM<-matrix((rbind(rqa1$LAM,rqa2$LAM,rqa3$LAM,rqa5$LAM,rqa6$LAM,rqa7$LAM )),6,1, byrow=T);"
                          "MatrizTT<-matrix((rbind(rqa3$TT)),1,1, byrow=T);"

//NA(Not Available): é uma constante lógica de tamanho 1 que contém um indicador de valor ausente.
//A função genérica is.na indica quais elementos estão faltando.
//A função genérica is.na<- define elementos para NA.

//na sequencia, identifica-se em todas linhas com valore inexistentes, e atribui 0;
//NOTA:talvez seja interessante saber os casos que podem ocorrer um valor "inexistente"
                          "MatrizRR[is.na(MatrizRR)]<-0;"
                          "MatrizDET[is.na(MatrizDET)]<-0;"
                          "MatrizMaxL[is.na(MatrizMaxL)]<-0;"
                          "MatrizL[is.na(MatrizL)]<-0;"
                          "MatrizENTR[is.na(MatrizENTR)]<-0;"
                          "MatrizLAM[is.na(MatrizLAM)]<-0;"
                          "MatrizTT[is.na(MatrizTT)]<-0;"

//A funçao "as.matrix" retorna todos os valores de um objeto Raster * como uma matriz.
//Para RasterLayers, linhas e colunas na matriz representam linhas e colunas no objeto RasterLayer.
//Para outros objetos Raster *, a matriz retornada por as.matrixtem colunas para cada camada e linhas para cada célula.
                          "MatrizRR <- as.matrix(MatrizRR);"
                          "MatrizDET <- as.matrix(MatrizDET);"
                          "MatrizMaxL <- as.matrix(MatrizMaxL);"
                          "MatrizL <- as.matrix(MatrizL);"
                          "MatrizENTR <- as.matrix(MatrizENTR);"
                          "MatrizLAM <- as.matrix(MatrizLAM);"
                          "MatrizTT <- as.matrix(MatrizTT);"

//------------------abaixo inicializa começa a clusterizaçao-------------------------------//

//----------------------lembretes sobre o processo de clusterizaçao-----------------------//
//O algoritmo K-means adaptativo é bastante simples
//1. Defina min.k e max.k.
//2. Execute K-médias com K = min.k
//3. Para cada cluster, verifique a condição de limite.
//4. Se todos os clusters satisfizerem a condição limite => Concluído, retorne o resultado
//5. Marque K> max.k => Se sim, pare. Se não, vá para o passo 5.
//6. Para qualquer cluster violando a condição de limite, execute K'-means com K '= 2 nesses membros de cluster,
//o que significa que K aumentará pelo número de clusters em violação. 7. Execute K-means definindo os centros de cluster atuais
//como os centros iniciais e vá para a etapa 4 .

//----------------------parametros de execuçao:----------------------------------------//
//MatrizRR= matriz de dados N por P, todos elementos devem ser numericos
//d.metric = 1: use métrica de distância euclidiana, caso contrário use métrica de distância de cosseno
//ths4=limiar para decidir se deseja aumentar k ou não: verificar todos os componentes da soma (abs (centro designado da amostra)) <ths4
//porque e melhor escolher esse parametro?
//min.k = minimo de cluster para iniciar
//mode= indica qual tipo de trasholde usar..1: use ths1, 2: use ths2, 3: use ths3

                          "require(akmeans);"
                          "AkmRR=akmeans(MatrizRR,d.metric=1, ths4=16, min.k = 1, mode=4);"
                          "AkmDET=akmeans(MatrizDET,d.metric=1, ths4=22, min.k = 1, mode=4);"
                          "AkmMaxL=akmeans(MatrizMaxL,d.metric=1, ths4=12, min.k = 1, mode=4);"
                          //AkmMaxL=akmeans(MatrizMaxL,d.metric=1, ths4=3, min.k = 1, mode=4);"
                          "AkmENTR=akmeans(MatrizENTR,d.metric=1, ths4=0.7, min.k = 1, mode=4);"
                          "AkmLAM=akmeans(MatrizLAM,d.metric=1, ths4=18, min.k = 1, mode=4);";
        R.parseEval(cmd4);// eval a string, ignorando qualquer retorno
        string str113 = "atrib<-AkmRR$cluster[]";
        Rcpp::NumericVector atrib = R.parseEval(str113);
        for ( x=0; x< atrib.size(); x++){
            cout << "atrib" << x<< "é:" << atrib[x] << endl;//encaminha LP TW S1 ao C++;
        }
        string str114 = "atrib2<-AkmDET$cluster[]";
        Rcpp::NumericVector atrib2 = R.parseEval(str114);
        string str115 = "atrib3<-AkmMaxL$cluster[]";
        Rcpp::NumericVector atrib3 = R.parseEval(str115);
        string str117 = "atrib5<-AkmENTR$cluster[]";
        Rcpp::NumericVector atrib5 = R.parseEval(str117);
        string str118 = "atrib6<-AkmLAM$cluster[]";
        Rcpp::NumericVector atrib6 = R.parseEval(str118);
        string str55 = "clusters<-AkmRR$centers[]";
        Rcpp::NumericVector clusters = R.parseEval(str55);

        for ( i=0; i< clusters.size(); i++){
            cout << "centroide" << i<< "é:" << clusters[i] << endl;//encaminha LP TW S1 ao C++;
        }
        string str56 = "clusters2<-AkmDET$centers[]";
        Rcpp::NumericVector clusters2 = R.parseEval(str56);
        string str57 = "clusters3<-AkmMaxL$centers[]";
        Rcpp::NumericVector clusters3 = R.parseEval(str57);
        string str58 = "clusters4<-MatrizL[1,1]";
        Rcpp::NumericVector clusters4 = R.parseEval(str58);
        string str59 = "clusters5<-AkmENTR$centers[]";
        Rcpp::NumericVector clusters5 = R.parseEval(str59);
        string str60 = "clusters6<-AkmLAM$centers[]";
        Rcpp::NumericVector clusters6 = R.parseEval(str60);
        string str61 = "clusters7<-MatrizTT[1,1]";
        Rcpp::NumericVector clusters7 = R.parseEval(str61);

        string str62 = "MQR1<-MatrizRR[]";
        Rcpp::NumericVector MQR1 = R.parseEval(str62);
        string str63 = "MQR2<-MatrizRR[]";
        Rcpp::NumericVector MQR2 = R.parseEval(str63);
        string str64 = "MQR3<-MatrizRR[]";
        Rcpp::NumericVector MQR3 = R.parseEval(str64);
        string str65 = "MQR5<-MatrizRR[]";
        Rcpp::NumericVector MQR5 = R.parseEval(str65);
        string str66 = "MQR6<-MatrizRR[]";
        Rcpp::NumericVector MQR6 = R.parseEval(str66);
        // inicio do bloco para testar os valores dos calculos de cada mqr, nesse bloco será efetuado um conjunto de sete testes condicionais
        //os valores testados são obtidos através de um  do conjunto de treinamento que é calculado suas mqr's, posteriormente submetidas ao akmeans, e obtido os valores dos centroids de cada cluster; 

        int c1=0;
        int x=0;
        int i=0;
        int cont=0;

        i=atrib[x]-1;
        if(clusters[i]<= 5.16){
            c1++;
        }
        if (MQR1[0]<= 5.16){
            c1++;
        }
        x++;
        i=atrib[x]-1;
        if (clusters[i]<= 4.35){
            c1++;

        }
        if (MQR1[1]<= 4.35){
            c1++;
        }
        x++;
        i=atrib[x]-1;
        if (clusters[i]<= 28.08){
            c1++;

        }

        if (MQR1[2]<= 28.08){
            c1++;
        }
        x++;
        i=atrib[x]-1;
        if (clusters[i]<= 21.70){
            c1++;
        }
        if (MQR1[3]<= 21.70){
            c1++;
        }
        x++;
        i=atrib[x]-1;
        if(clusters[i]<= 4.44){
            c1++;
        }
        if(MQR1[4]<= 4.44){
            c1++;
        }
        x++;
        i=atrib[x]-1;
        if(clusters[i]<= 3.08){
            c1++;
        }
        if(MQR1[5]<= 3.08){
            c1++;
        }
        x++;
        i=atrib[x]-1;
        if (clusters[i]<= 4.35){
            c1++;
        }
        if (MQR1[6]<= 4.35){
            c1++;
        }
        if (c1<7){
            salva_anomalia("saida1.txt");
            cont++;
        }else{
            salva_normalidade("saida1.txt");
        }
        printf("%d\n",c1);
        //--------------------fim do primeiro conjunto de comparaçoes--------------------//

        //zera as variavies auxiliares
        c1=0;
        x=0;
        i=0;
        i=atrib2[x]-1;
        if( clusters2[i] <= 41.33){
            c1++;
        }
        if( MQR2[0] <= 41.33){
            c1++;
        }
        x++;
        i=atrib2[x]-1;
        if( clusters2[i] <= 47.32){
            c1++;
        }
        if( MQR2[1] <= 47.32){
            c1++;
        }
        x++;
        i=atrib2[x]-1;
        if( clusters2[i] <= 82.52){
            c1++;
        }
        if( MQR2[2] <= 82.52){
            c1++;
        }
        x++;
        i=atrib2[x]-1;
        if( clusters2[i] <= 69.08){
            c1++;
        }
        if( MQR2[3] <= 69.08){
            c1++;
        }
        x++;
        i=atrib2[x]-1;
        if( clusters2[i] <= 43.74){
            c1++;
        }
        if( MQR2[4] <= 43.74){
            c1++;
        }
        x++;
        i=atrib2[x]-1;
        if( clusters2[i] <= 42.20){
            c1++;
        }
        if( MQR2[5] <= 42.20){
            c1++;
        }
        x++;
        i=atrib2[x]-1;
        if( clusters2[i] <= 47.33){
            c1++;
        }
        if( MQR2[6] <= 47.33){
            c1++;
        }
        if (c1<8){
            salva_anomalia("saida2.txt");
            cont++;
        }else{
            salva_normalidade("saida2.txt");
        }
        printf("%d\n",c1);
        //--------------------fim do segundo conjunto de comparaçoes--------------------//

        //zera as variavies auxiliares
        c1=0;
        x=0;
        i=0;
        i=atrib3[x]-1;
        if( clusters3[i] <= 4.83){
            c1++;
        }
        if( MQR3[0] <= 4.83){
            c1++;
        }
        x++;
        i=atrib3[x]-1;
        if( clusters3[i] <= 4.74){
            c1++;
        }
        if( MQR3[1] <= 4.74) {
            c1++;
        }
        x++;
        i=atrib3[x]-1;
        if( clusters3[i] <= 13.87){
            c1++;
        }
        if( MQR3[2] <= 13.87){
            c1++;
        }
        x++;
        i=atrib3[x]-1;
        if( clusters3[i] <= 4.43){
            c1++;
        }
        if( MQR3[3] <= 4.43){
            c1++;
        }
        x++;
        i=atrib3[x]-1;
        if( clusters3[i] <= 4.17){
            c1++;
        }
        if( MQR3[4] <= 4.17){
            c1++;
        }
        x++;
        i=atrib3[x]-1;
        if( clusters3[i] <= 4.74){
            c1++;
        }
        if( MQR3[5] <= 4.74){
            c1++;
        }
        if (c1<6){
            salva_anomalia("saida3.txt");
            cont++;
        }else{
            salva_normalidade("saida3.txt");
        }
        printf("%d\n",c1);
        //--------------------fim do terceiro conjunto de comparaçoes--------------------//

        //zera as variavies auxiliares
        x=0;
        i=0;
        if( clusters4[i] > 3.57){
            salva_anomalia("saida4.txt");
            cont++;
        }else{
            salva_normalidade("saida4.txt");
        }
        //--------------------fim do quarto conjunto de comparaçoes--------------------//

        //zera as variavies auxiliares
        c1=0;
        x=0;
        i=0;
        i=atrib5[x]-1;
        if( clusters5[i] <= 0.88){
            c1++;
        }
        if( MQR5[0] <= 0.88){
            c1++;
        }
        x++;
        i=atrib5[x]-1;
        if( clusters5[i] <= 0.85){
            c1++;
        }
        if( MQR5[1] <= 0.85){
            c1++;
        }
        x++;
        i=atrib5[x]-1;
        if( clusters5[i] <= 1.64){
            c1++;
        }
        if( MQR5[2] <= 1.64){
            c1++;
        }
        x++;
        i=atrib5[x]-1;
        if( clusters5[i] <= 1.22){
            c1++;
        }
        if( MQR5[3] <= 1.22){
            c1++;
        }
        x++;
        i=atrib5[x]-1;
        if( clusters5[i] <= 0.78){
            c1++;

        }
        if( MQR5[4] <= 0.78){
            c1++;
        }
        x++;
        i=atrib5[x]-1;
        if( clusters5[i] <= 0.77){
            c1++;
        }
        if( MQR5[5] <= 0.77){
            c1++;
        }
        x++;
        i=atrib5[x]-1;
        if( clusters5[i] <= 0.85){
            c1++;
        }
        if( MQR5[6] <= 0.85){
            c1++;
        }
        if (c1<7){
            salva_anomalia("saida5.txt");
            cont++;
        }else{
            salva_normalidade("saida5.txt");
        }
        //--------------------fim do quinto conjunto de comparaçoes--------------------//

        //zera as variavies auxiliares
        c1=0;
        x=0;
        i=0;
        i=atrib6[x]-1;
        if( clusters6[i] <= 40.10){
            c1++;
        }
        if( MQR6[0] <= 40.10){
            c1++;
        }
        x++;
        i=atrib6[x]-1;
        if( clusters6[i] <= 37.88){
            c1++;
        }
        if( MQR6[1] <= 37.88){
            c1++;
        }
        x++;
        i=atrib6[x]-1;
        if( clusters6[i] <= 82.83){
            c1++;
        }
        if( MQR6[2] <= 82.83){
            c1++;
        }
        x++;
        i=atrib6[x]-1;
        if( clusters6[i] <= 32.78){
            c1++;
        }
        if( MQR6[3] <= 32.78){
            c1++;
        }
        x++;
        i=atrib6[x]-1;
        if( clusters6[i] <= 31.46){
            c1++;
        }
        if( MQR6[4] <= 31.46){
            c1++;
        }
        x++;
        i=atrib6[x]-1;
        if( clusters6[i] <= 37.88){
            c1++;
        }
        if( MQR6[5] <= 37.88){
            c1++;
        }
        if (c1<6){
            salva_anomalia("saida6.txt");
            cont++;
        }else{
            salva_normalidade("saida6.txt");
        }
        //--------------------fim do sexto conjunto de comparaçoes--------------------//

        //zera as variavies auxiliares
        x=0;
        i=0;

        if( clusters7[i] > 4.32 ){
            salva_anomalia("saida7.txt");
            cont++;
        }else{
            salva_normalidade("saida7.txt");
        }
        if (cont>=4){
            salva_anomalia("saidageral.txt");

        }else{
           salva_normalidade("saidageral.txt");
        }

    }
    while (aux<n_linhas);
//return 0;
}
