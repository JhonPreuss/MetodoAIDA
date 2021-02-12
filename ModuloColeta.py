"""
Módulo Coleta e Segmentação - Método AIDA 
Função básica do Módulo Coleta e Segmentação é coletar o tráfego bruto de rede IoT, extrair atributos de interesse e convertê-los em séries de dados
organizados para serem utilizadas no processo de segmentação (identificação da classe comportamental do dispositivo) e no processo de detecção (classificação do comportamento normal ou anômalo).

A fase de coleta de dados corresponde a captura do tráfego da rede do ambiente IoT  durante uma janela de tempo previamente estabelecida.
A janela de tempo pode ser diferente para cada ambiente IoT.

Com esse módulo do tráfego amostrado são extraídos dois conjuntos de atributos: um que é usado para determinar as classes dos dispositivos (segmentação) e outro para ser usado na análise 
de comportamento anômalo (detecção).

- Cada série de dados resultante do Módulo Coleta e Segmentação (séries S1, S2, S3, ..., Sn), e que será usada pela AQR, pertence a um dispositivo já associado a uma classe de dispositivos IoT 
presentes na rede analisada.

- Quando o dispositivo não é conhecido, aplica-se o processo de segmentação usando um classificador k-means.[TO DO]
    - Nessa versão do módulo essa função não está implementada.

Funcionamento :
- Para cada classe comportamental do dispositivo presente em uma rede IoT, deve se criar uma instancia desse módulo.
- 
- Esse deamon, pode operar de duas formas pode ser instanciado de duas formas:
-- Monitoramento de interface  de rede
--- Passando três parâmetros, Setamdo como parâmetro a interface de rede que sera monitorada para extração das caracteristicas de tráfego, o conjunto de  enderços MAC dos dispositivos de uma determinada classe comoprtamental 
    e a janela de tempo aonde será caputado o tráfego da rede e o arquivo de saída para os dados 
--- Nesse modo de operação o deamon deve ser executado junto ao gateway IoT.

-- Extração de dados apartir de .pcap
-- Passando três parâmetros, Setamdo como parâmetro o arquivo .pcap para extração das caracteristicas de tráfego, o conjunto de  enderços MAC dos dispositivos de uma determinada classe comoprtamental 
e a janela de tempo aonde será caputado o tráfego da rede e o arquivo de saída para os dados
"""
#Conjunto de bibliotecas para o funcionamento do módulo de coleta 
from scapy.all import *
from threading import Thread, Event
import time
import datetime
import dpkt
from time import sleep
import logging
import csv
import traceback
import pandas as pd
import shutil 
import os.path
from os import path


# definição do arquivo pcap  contendo o tráfego de rede
pcap_ler="16-10-01_14h_20h.pcap" 
# definição do diretório de saída 
caminho_arquivo="[diretório]"
#variavel auxiliar usada para renomear o arquivo de saída
hora_pcap="_14_20"
#nos outros coletor desabilitar pq ja vai existir o arquivo basico .csv
source1 = "[diretório]"
#variáveis de auxilo para manipulação do pcap
#indicando o tempo de o minuto e seg de inicio, usada por causa que eventualmente a biblioteca que manipula pcap perde a referencia do tempo 
min_inicial=0
sec_inicial=1

#Definição dos endereços mac e nome para idenficação 
Dispositivo = ['Smart_Things','Amazon_Echo','Netatmo_Welcome','TP-Link_Day_Night_Cloud_camera','Samsung_SmartCam','Dropcam','Insteon_Camera','Insteon_Camera2','Withings_Smart_Baby_Monitor','Belkin_Wemo_switch','TP-Link_Smart_plug','iHome','Belkin_wemo_motion_sensor','NEST_Protect_smoke_alarm','Netatmo_weather_station','Withings_Smart_scale','Blipcare_Blood_Pressure_meter','Withings_Aura_smart_sleep_sensor','Light_Bulbs_LiFX_Smart_Bulb','Triby_Speaker','PIX-STAR_Photo-frame','HP_Printer','Samsung_Galaxy_Tab','Nest_Dropcam','Android_Phone','MacBook','Android_Phone2','IPhone','MacBook/Iphone','TPLink_Router_Bridge']
Mac = ['d0:52:a8:00:67:5e','44:65:0d:56:cc:d3','70:ee:50:18:34:43','f4:f2:6d:93:51:f1','00:16:6c:ab:6b:88','30:8c:fb:2f:e4:b2','00:62:6e:51:27:2e','e8:ab:fa:19:de:4f','00:24:e4:11:18:a8','ec:1a:59:79:f4:89','50:c7:bf:00:56:39','74:c6:3b:29:d7:1d','ec:1a:59:83:28:11','18:b4:30:25:be:e4','70:ee:50:03:b8:ac','00:24:e4:1b:6f:96','74:6a:89:00:2e:25','00:24:e4:20:28:c6','d0:73:d5:01:83:08','18:b7:9e:02:20:44','e0:76:d0:33:bb:85','70:5a:0f:e4:9b:c0','08:21:ef:3b:fc:e3','30:8c:fb:b6:ea:45','40:f3:08:ff:1e:da','ac:bc:32:d4:6f:2f','b4:ce:f6:a7:a3:c2','d0:a6:37:df:a1:e1','f4:5c:89:93:cc:85','14:cc:20:51:33:ea']

#Inicialização das varíavies que serão inseridas nas colunas no arquivo de saída csv, nem todos campos são utilizados para AQR
count = 0
count_src= 0
count_dst= 0
contador= 0
contador_tcp= 0
contador_udp= 0
contador_resto= 0
mais_usado=0
con_s_disp =0


# linha só para debug em execução, como o arquivo de saída é padrão, utilizei um arquivo apenas com o cabeçalho das colunas, e para cada grupo/dispositivo o programa faz uma cópia do arqui em branco e renomeia
#antes de iniciar a adição dos dados coletados
print("[*]copia e  renomeia csv inicial")
#----------------------------------------------------
# pego o tamanho da lista de dispositivos, para saber quantos dispositivos tem a classe comportamental que está sendo analisada
length = len(Dispositivo)
# esse valor será de usado como tamanho do interador do laço que irá criar os arquivos de saída coletadas do tráfego
for i in range(length):
    destination1 = "{caminho}{arquivo}.csv".format(caminho=caminho_arquivo, arquivo=Dispositivo[i])
    cria_dir_dvce = "{caminho}{arquivo}".format(caminho=caminho_arquivo, arquivo=Dispositivo[i])


    #testa se a pasta do diretório do dispositivo informado existe
    if not path.exists(cria_dir_dvce):
        access_rights = 0o777
        try:
            os.mkdir(cria_dir_dvce, access_rights)
        except OSError:
            print ("Falha ao criar o diretório %s" % cria_dir_dvce)
        else:
            print ("Diretório %s criado com sucesso" % cria_dir_dvce)

    else:
        print("Erro no teste de criar diretório local.")
    if not path.exists(destination1):
        #testa se não existir arquivo copiado da base.csv
        try:
            shutil.copy(source1, destination1) 
            print("Arquivo copiádo com sucesso.")
            csv_nome=destination1
        #If source and destination are same 
        except shutil.SameFileError: 
            print("Origem e destino representam o mesmo arquivo.") 
        # If there is any permission issue 
        except PermissionError: 
            print("Permissão negada.") 
        # For other errors 
        except: 
            print("Ocorreu um erro ao copiar o arquivo.")  
            print("{src}:{dst}".format(src=Dispositivo[i], dst=Mac[i]))
    else:
        print("Erro  no teste de arquivo local.") 
        csv_nome=destination1

        #---------------------------------------------
    mac=Mac[i]
    #função utilizada para gravar em forma de append o conteúdo de uma lista em um arquivo csv.
    def WriteCsv(list_lists, csvFile):
        try:
            #recebe a lista de registros e o arquivo que sera gravado
            file = pd.read_csv(csvFile)
            out_csv = pd.DataFrame(list_lists)
            file=file.append(out_csv)
            out_csv.to_csv(csvFile, mode='a', index=False, header=False)
            return True
        except Exception as g:
            x=str(g)
            return x


#função CORE, nessa função é realizada a captura e analise dos pacotes (seja eles vindos de pcap ou interface)
    def print_packet(packet):
    #-Defição de veriaveis globais que abrigaram valores capturados pela biblioteca do wireshark
        global count
        global contador
        global contador_tcp
        global contador_udp
        global contador_resto
        global mais_usado
        global count_src
        global count_dst
        global min_inicial
        global sec_inicial
        global con_s_disp        
        global lst_features_pkt
        global lst_timestemp
        global lst_tmp
        global lst_min
        global lst_total_pg
        global lst_total_dpg
        global lst_total_pgN
        global lst_contador_tcp
        global lst_contador_udp
        global lst_contador_outros
        global lst_tmp_s_disp
        global lst_min_s_disp
        global lst_timestemp_s_disp
        global lst_ZERO
    #-Definição de listas que receberão os valores  capturados pela biblioteca do wireshark formando 
    # essas lista servem para evitar o processo de abertura e gravação em arquivo multiplas vezes, pelo fato de desse processo ser custoso computacionalmente para hardwares menores como um roteador.
    # ao final de uma janela de tempo, essas listas serão gravadas no arquivo de saída todas de uma única vez
        lst_features_pkt=[]
        lst_timestemp=[]
        lst_tmp=[]
        lst_min=[]
        lst_total_pgN=[]
        lst_contador_tcp=[]
        lst_contador_udp=[]
        lst_contador_outros=[]
        lst_tmp_s_disp=[]
        lst_min_s_disp=[]
        lst_timestemp_s_disp=[]
        lst_ZERO=[]
        lst_ZERO.append(0)

        #esse comando pegara todos os campos das camada IP dos pacotes ethernet
        ip_layer = packet.getlayer(IP)
        #converte o formato de horario vindo do pacote, para um formato mais legivel
        human_time=datetime.datetime.utcfromtimestamp(packet.time).strftime('%Y-%m-%d %H:%M:%S:%f')
        
        #variáveis de minuto e segundo para calcular o inicio e fim do intevalo usado  para cada janela de tempo
        t_min=datetime.datetime.utcfromtimestamp(packet.time).strftime('%M')
        t_s=datetime.datetime.utcfromtimestamp(packet.time).strftime('%S')

        print(human_time)
        devices_group=[mac]#macdispositivo
        
        #Testa se no pacote analisado é um pacote do tipo Ethernet, para garantir que não seja analisados pacotes de outro tipo outra estrutura
        if Ether in packet:
            #pega os campos da camada IP
            ip_layer = packet.getlayer(IP)
            #pega os campos da camada Ethernet
            Ether_layer = packet.getlayer(Ether)
            
            #Testa se o pacote analisado é destinado a um dispositivo da classe comportamental a ser analisada
            if Ether_layer.dst in devices_group or Ether_layer.src in devices_group:
                #contador de pacotes tcp, dentro do intervalo da janela de tempo, aqui é 1min
                if min_inicial+1 != int(t_min):
                    contador +=1
                    #teste se é tcp ou udp
                    if TCP in packet:
                        print("TCP")
                        contador_tcp+=1
                    if UDP in packet:
                        print("UDP")
                        contador_udp+=1            
                    else:
                        print("Outro tipo de pacotote")
                        contador_resto+=1

                else:
                    #calculo para verificar qual o protoco mais usado durante aquela janela de tempo analisada
                    # durante a a anlise, se o maior número de pacotes for do tipo TCP, a variavel "mais_usado" recebe um valor indicativo qualquer, aqui é adotado 6.
                    # caso o maior número de pacores for do tipo UDP a variavel "mais_usado" recebe um valor indicativo qualquer, aqui é adotado 17.
                    mais_usado = 6 if contador_tcp >= contador_udp else 17

                    #Testa o tempo para verificar se chegou ao fim da janela de tempo, caso tenha chego, deve-se gravar os dados já coletados no arquivo de saída
                    if int(t_min)== 59:
                        # reinicia a variavel de tempo
                        min_inicial=0


                        # ----------------------------------------------------------                 
                        lst_timestemp.append(contador)
                        lst_min.append(min_inicial)
                        lst_tmp.append(human_time)
                        lst_total_pgN.append(mais_usado)
                        lst_contador_tcp.append(contador_tcp)
                        lst_contador_udp.append(contador_udp)
                        lst_contador_outros.append(contador_resto)

                        #começa a ordenar as listas para adicionar o csv
                        lst_features_pkt.append(lst_tmp)#tempo com 1 minuto de atraso
                        lst_features_pkt.append(lst_min)#minuto atual
                        lst_features_pkt.append(lst_timestemp)#contador de pacotes no minuto atual
                        lst_features_pkt.append(lst_total_pgN)#classificação protocolo mais aparente 
                        lst_features_pkt.append(lst_contador_tcp)
                        lst_features_pkt.append(lst_contador_udp)#
                        lst_features_pkt.append(lst_contador_outros)

                        #prepara as multiplas listas com os dados capturados, para gravar cada lista em uma coluna do CSV
                        zippedList=list(zip(lst_features_pkt[0],lst_features_pkt[1],lst_features_pkt[2],lst_features_pkt[3],lst_features_pkt[4],lst_features_pkt[5],lst_features_pkt[6]))
                        retrun_write=WriteCsv(zippedList, csv_nome)#nomecsv

                        #Processo para limpar todas as listas após os dados serem gravados no arquivo de saída.
                        lst_features_pkt.clear()
                        lst_contador_outros.clear()
                        lst_total_pgN.clear()
                        lst_contador_tcp.clear()
                        lst_contador_udp.clear()

                        #Zera todas as variavis utilizadas como auxiliar para contadores 
                        contador_udp=contador_resto=contador_tcp=mais_usado=0
                        contador=1

                        #--------TRATAMENTO DE MINUTO DE TRANZAÇÃO (59 para 00) ---------------#
                        if TCP in packet:#teste se é tcp ou udp
                            #tcp_layer=packet.getlayer(TCP)
                            print("TCP")
                            contador_tcp+=1

                        if UDP in packet:
                            #udp_layer = packet.getlayer(UDP)
                            print("UDP")
                            contador_udp+=1
                                            
                        else:
                            print("Resto")
                            contador_resto+=1
                    else:
                        min_inicial=int(t_min)
                        # ----------------------------------------------------------                 
                        lst_timestemp.append(contador)
                        lst_min.append(min_inicial)
                        lst_tmp.append(human_time)
                        lst_total_pgN.append(mais_usado)
                        lst_contador_tcp.append(contador_tcp)
                        lst_contador_udp.append(contador_udp)
                        lst_contador_outros.append(contador_resto)
                        #começa a ordenar as listas para adicionar o csv
                        lst_features_pkt.append(lst_tmp)#tempo com 1 minuto de atraso
                        lst_features_pkt.append(lst_min)#minuto atual
                        lst_features_pkt.append(lst_timestemp)#contador de pacotes no minuto atual
                        lst_features_pkt.append(lst_total_pgN)#classificação protocolo mais aparente 
                        lst_features_pkt.append(lst_contador_tcp)
                        lst_features_pkt.append(lst_contador_udp)#
                        lst_features_pkt.append(lst_contador_outros)
                        zippedList=list(zip(lst_features_pkt[0],lst_features_pkt[1],lst_features_pkt[2],lst_features_pkt[3],lst_features_pkt[4],lst_features_pkt[5],lst_features_pkt[6]))
                        retrun_write=WriteCsv(zippedList, csv_nome)#nomecsv
                        lst_features_pkt.clear()
                        lst_contador_outros.clear()
                        lst_total_pgN.clear()
                        lst_contador_tcp.clear()
                        lst_contador_udp.clear()
                        contador_udp=contador_resto=contador_tcp=mais_usado=0
                        contador=1
                        #limpa todas variaveis
                        #--------TRATAMENTO DE MINUTO DE TRANZAÇÃO ---------------#
                        if TCP in packet:#teste se é tcp ou udp
                            #tcp_layer=packet.getlayer(TCP)
                            print("TCP")
                            contador_tcp+=1

                        if UDP in packet:
                            #udp_layer = packet.getlayer(UDP)
                            print("UDP")
                            contador_udp+=1
                                            
                        else:
                            print("Resto")
                            contador_resto+=1

            #Caso o pacote analisado não for do tipo Etherne, o processo de contabilização de pacotes é semelhante, avançando apenas no tempo da janela em analise
            else:
                if min_inicial+1 != int(t_min):
                    print("[!]-[!] pacote ip não encontrado na lista de macs")
                else:
                    #vai gravar o que tiver na lista até o momento, em caso dos pacotes analisados estiverem ao fim da janela de tempo analisada
                    print("[!]-[!] pacote ip não eencontrado na lista de macs")

                    #---------calculo-------------------------------------------
                    mais_usado = 6 if contador_tcp >= contador_udp else 17

                    if int(t_min)== 59:
                        min_inicial=0
                                      
                        lst_timestemp.append(contador)
                        lst_min.append(min_inicial)
                        lst_tmp.append(human_time)
                        lst_total_pgN.append(mais_usado)
                        lst_contador_tcp.append(contador_tcp)
                        lst_contador_udp.append(contador_udp)
                        lst_contador_outros.append(contador_resto)
                        #começa a ordenar as listas para adicionar o csv
                        lst_features_pkt.append(lst_tmp)#tempo com 1 minuto de atraso
                        lst_features_pkt.append(lst_min)#minuto atual
                        lst_features_pkt.append(lst_timestemp)#contador de pacotes no minuto atual
                        lst_features_pkt.append(lst_total_pgN)#classificação protocolo mais aparente 
                        lst_features_pkt.append(lst_contador_tcp)
                        lst_features_pkt.append(lst_contador_udp)#
                        lst_features_pkt.append(lst_contador_outros)
                        zippedList=list(zip(lst_features_pkt[0],lst_features_pkt[1],lst_features_pkt[2],lst_features_pkt[3],lst_features_pkt[4],lst_features_pkt[5],lst_features_pkt[6]))
                        retrun_write=WriteCsv(zippedList, csv_nome)#nomecsv
                        lst_features_pkt.clear()
                        lst_contador_outros.clear()
                        lst_total_pgN.clear()
                        lst_contador_tcp.clear()
                        lst_contador_udp.clear()
                        contador_udp=contador_resto=contador_tcp=mais_usado=0
                        contador=1
                        #limpa todas variaveis
                        #--------TRATAMENTO DE MINUTO DE TRANZAÇÃO ---------------#
                    else:
                        min_inicial=int(t_min)
                        # ----------------------------------------------------------                 
                        lst_timestemp.append(contador)
                        lst_min.append(min_inicial)
                        lst_tmp.append(human_time)
                        lst_total_pgN.append(mais_usado)
                        lst_contador_tcp.append(contador_tcp)
                        lst_contador_udp.append(contador_udp)
                        lst_contador_outros.append(contador_resto)
                        
                        #Começa a ordenar as listas para adicionar o csv
                        lst_features_pkt.append(lst_tmp)#tempo com 1 minuto de atraso
                        lst_features_pkt.append(lst_min)#minuto atual
                        lst_features_pkt.append(lst_timestemp)#contador de pacotes no minuto atual
                        lst_features_pkt.append(lst_total_pgN)#classificação protocolo mais aparente 
                        lst_features_pkt.append(lst_contador_tcp)
                        lst_features_pkt.append(lst_contador_udp)#
                        lst_features_pkt.append(lst_contador_outros)

                        zippedList=list(zip(lst_features_pkt[0],lst_features_pkt[1],lst_features_pkt[2],lst_features_pkt[3],lst_features_pkt[4],lst_features_pkt[5],lst_features_pkt[6]))
                        retrun_write=WriteCsv(zippedList, csv_nome)#nomecsv

                        #Limpa todas variaveis
                        lst_features_pkt.clear()
                        lst_contador_outros.clear()
                        lst_total_pgN.clear()
                        lst_contador_tcp.clear()
                        lst_contador_udp.clear()
                        contador_udp=contador_resto=contador_tcp=mais_usado=0
                        contador=1
                        

        #Caso o pacote do tipo IP não for encontrado.
        #Deve-se inicar os teste para verificar o tempo da janela de caputra e em caso de estar no fim
        # gravar o que tiver nas listas até o momento, em caso dos pacotes analisados estiverem ao fim da janela de tempo analisada
        else:
            print("[!]-[!] Pacote IP nao encontrado")
            #time.sleep(1)
            if min_inicial+1 != int(t_min):
                print("[!]-[!] if Caiu no else de pacote ip nao encontrado:")
            else:
                #time.sleep(5)
                print("Gravar no else de pacote ip não encontrado")
                mais_usado = 6 if contador_tcp >= contador_udp else 17

                if int(t_min)== 59:
                        min_inicial=0
                        # ----------------------------------------------------------                 
                        lst_timestemp.append(contador)
                        lst_min.append(min_inicial)
                        lst_tmp.append(human_time)
                        lst_total_pgN.append(mais_usado)
                        lst_contador_tcp.append(contador_tcp)
                        lst_contador_udp.append(contador_udp)
                        lst_contador_outros.append(contador_resto)
                        #começa a ordenar as listas para adicionar o csv
                        lst_features_pkt.append(lst_tmp)#tempo com 1 minuto de atraso
                        lst_features_pkt.append(lst_min)#minuto atual
                        lst_features_pkt.append(lst_timestemp)#contador de pacotes no minuto atual
                        lst_features_pkt.append(lst_total_pgN)#classificação protocolo mais aparente 
                        lst_features_pkt.append(lst_contador_tcp)
                        lst_features_pkt.append(lst_contador_udp)#
                        lst_features_pkt.append(lst_contador_outros)
                        zippedList=list(zip(lst_features_pkt[0],lst_features_pkt[1],lst_features_pkt[2],lst_features_pkt[3],lst_features_pkt[4],lst_features_pkt[5],lst_features_pkt[6]))
                        retrun_write=WriteCsv(zippedList, csv_nome)#nomecsv
                        lst_features_pkt.clear()
                        lst_contador_outros.clear()
                        lst_total_pgN.clear()
                        lst_contador_tcp.clear()
                        lst_contador_udp.clear()
                        contador_udp=contador_resto=contador_tcp=mais_usado=0
                        contador=1
                        #limpa todas variaveis

                else:
                        min_inicial=int(t_min)
                        # ----------------------------------------------------------                 
                        lst_timestemp.append(contador)
                        lst_min.append(min_inicial)
                        lst_tmp.append(human_time)
                        lst_total_pgN.append(mais_usado)
                        lst_contador_tcp.append(contador_tcp)
                        lst_contador_udp.append(contador_udp)
                        lst_contador_outros.append(contador_resto)
                        #começa a ordenar as listas para adicionar o csv
                        lst_features_pkt.append(lst_tmp)#tempo com 1 minuto de atraso
                        lst_features_pkt.append(lst_min)#minuto atual
                        lst_features_pkt.append(lst_timestemp)#contador de pacotes no minuto atual
                        lst_features_pkt.append(lst_total_pgN)#classificação protocolo mais aparente 
                        lst_features_pkt.append(lst_contador_tcp)
                        lst_features_pkt.append(lst_contador_udp)#
                        lst_features_pkt.append(lst_contador_outros)
                        zippedList=list(zip(lst_features_pkt[0],lst_features_pkt[1],lst_features_pkt[2],lst_features_pkt[3],lst_features_pkt[4],lst_features_pkt[5],lst_features_pkt[6]))
                        retrun_write=WriteCsv(zippedList, csv_nome)#nomecsv

                    #limpa todas variaveis
                        lst_features_pkt.clear()
                        lst_contador_outros.clear()
                        lst_total_pgN.clear()
                        lst_contador_tcp.clear()
                        lst_contador_udp.clear()
                        contador_udp=contador_resto=contador_tcp=mais_usado=0
                        contador=1
                        #--------TRATAMENTO DE MINUTO DE TRANZAÇÃO ---------------#
    print("[*] Inicio do processo de sniffing...")
    sniff(offline=pcap_ler, filter="ip", prn=print_packet)
    print("[*] Parada do processo de sniffing")
    print(contador)

    print("[*]Copiar e  ronomear o arquivo de saida csv")
    source=destination1
    destination = "{caminho}{arquivo}/{arquivo}{hora}.csv".format(caminho=caminho_arquivo, arquivo=Dispositivo[i], hora=hora_pcap)


    try: 
        shutil.copy(source, destination) 
        print("Arquivo copiado com sucesso.") 

    # Caso o arquivo de origem e destinho sejam o mesmo
    except shutil.SameFileError: 
        print("O arquivo de origem e destino da cópia são o mesmo .") 

    # se o deamon ou diretório não tiver permissão para a ação
    except PermissionError: 
        print("Permissão negada.") 

    # Demais erros
    except: 
        print("Erro durante a cópia do arquivo.")


