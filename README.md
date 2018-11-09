# Krawler MPI

Web crawler distribuído para sites de e-commerce

## Objetivo

Um *Web Crawler* tem como objetivo recuperar informações de uma ou mais páginas web de maneira inteligente, percorrendo caminhos criados pelos links de cada página. Neste projeto, o objetivo é criar um [Web Crawler](https://en.wikipedia.org/wiki/Web_crawler) capaz de recuperar informações de produtos disponíveis em sites de e-commerce e disponibilizá-las para o usuário, semelhante ao funcionamento do site [Buscapé](https://www.buscape.com.br/). Este *crawler* deve ser implementado de maneira distribuída, afim de diminuir o tempo de execução do programa, criando assim um sistema amigável para o usuário.

A biblioteca [MPI](https://www.open-mpi.org/) será utilizada para realizar esta implementação distribuída, mais especificamente a versão disponibilizada pelo framework [Boost](https://www.boost.org/). A distribuição do programa se dá por meio da utilização de diversos processos e troca de mensagens entre estes processos, ações facilitadas pelo MPI. O intuito deste tipo de implementação é permitir a execução do programa em sistemas distribuídos, como por exemplo um *cluster* no [AWS](https://docs.aws.amazon.com/AmazonECS/latest/developerguide/ECS_clusters.html).

## Dependências
- Boost
- MPI
- GCC/G++
- CMake
- Make

## MPI

O MPI (Message Passing Interface) é uma biblioteca criada para permitir o desenvolvimento de programas que utilizam diversos processos de um ou mais computadores para executar tarefas paralelamente. Este protocolo funciona com trocas de mensagens (dados) entre processos a partir de um processo mestre. Neste projeto, queremos fazer com que diversas URLs sejam acessadas ao mesmo tempo, portanto o objetivo do MPI será dividir URLs para cada processo, que por sua vez irão realizar a coleta de produtos e enviá-los a um processo principal (normalmente o 0 (zero)) para que sejam mostrados. os processos podem ser criados na mesma máquina ou em máquinas diferentes. Caso esteja rodando num *cluster*, um arquivo contendo hosts pode ser especificado. A utilização com *clusters* não será detalhada neste relatório, mas pode ser facilmente encontrada na Internet.

## Funcionamento

As tarefas realizadas por cada processo são bastante simples:
1. Realizar o download de páginas web.
2. Coletar informações de produtos utilizando [Expressões Regulares](https://en.wikipedia.org/wiki/Regular_expression).
3. Mostrar as informações de cada produto em formato [JSON](https://www.json.org/).

Primeiramente, o processo 0 (zero) recebe um lista com diversas URLs. Esta lista será quebrada quase igualmente entre N processos, definidos pelo usuário que executa o programa. Cada lista menor menor será enviada para um outro processo, onde as URLs serão percorridas e utilizadas para baixar diferentes produtos.

##### Exemplo
Suponha que a lista inicial de URLs possua 40 links distintos.
A lista inicial tem o seguinte formato:

```python
[url1, url2, url3, url4, ..., url40]
```

Suponha também que colocaremos 4 processos para serem executados. Uma nova lista será criada contendo 4 elementos. Cada elemento é um novo vetor com 10 URLs:

```python
[
    [url1,  ..., url10],
    [url11, ..., url20],
    [url21, ..., url31],
    [url31, ..., url40]
]
```

Após esta separação, utilizamos a função `boost::mpi::scatter()` para enviar cada elemento a um processo diferente. O primeiro vetor (URLs 1-10) será enviado para o processo de mesmo índice de sua posição na lista, ou seja, 0 (zero). Já o segundo vetor (URLs 11-20) será enviado para o processo 1, e assim por diante.

Cada processo utilizará seu conjunto de URLs para coletar os produtos, podendo navegar para outros links afim de facilitar a coleta. Cada produto será composto pelas seguintes informações: nome, descrição, foto, preço, preço parcelado, número de parcelas, categoria e URL específica do produto, formatados em um JSON conforme abaixo:

```json
{
    "nome": "Produto",
    "descricao": "Descrição do Produto",
    "foto": "http://...",
    "preco": 1000,
    "preco_parcelado": 1000,
    "preco_num_parcelas": 10,
    "categoria": "Categoria",
    "url": "http://..."
}
```

As informações são obtidas por meio do uso de expressões regulares que as detectam em meio ao conteúdo HTML de cada página. Após a finalização de todas as URLs de todos os processos, a função `boost::mpi::gather()` é utilizada para agrupar novamente os produtos gerados, em um processo escolhido pelo usuário (novamente, normalmente o 0 (zero)). Após agrupados, os produtos serão mostrados ao usuário via saída padrão (terminal). No final do relatório existe um diagrama detalhando a troca de mensagens entre os processos.

## Métricas

Afim de explorar a performance deste programa, algumas métricas de tempo são coletadas ao longo do *crawling*. São elas:

- Tempo ocioso (TOTAL_IDLE_TIME): tempo de espera pelo download de páginas
- Tempo por produto (PROD_TIME): tempo gasto para baixar uma página específica de um produto e criar sua visualização em JSON. Cada produto baixado possui um PROD_TIME.
- Tempo médio por produto (AVG_TIME_PER_PRODUCT): tempo total de execução dividido pelo número de produtos coletados.

Outras métricas coletadas:
- Tempo total de execução (TOTAL_EXEC_TIME).
- Número de produtos coletados (TOTAL_PROD_COUNT).

Todas as métricas são enviadas para a saída de erro padrão do sistema, que pode ser redirecionada para um arquivo de texto a gosto do usuário, como no exemplo a seguir (todos os valores estçao em segundos, com exceção de TOTAL_PROD_COUNT que é uma contagem):

```
PROD_TIME: 1.2
PROD_TIME: 0.85
PROD_TIME: 0.50
...
TOTAL_PROD_COUNT: 200
TOTAL_IDLE_TIME: 180
TOTAL_EXEC_TIME: 30
AVG_TIME_PER_PRODUCT: 0.7
```

## Utilização

#### Sites suportados
- [Magazine Luiza](https://www.magazineluiza.com.br/)

O programa utiliza categorias de produtos como entrada. A categoria é uma URL de um site suportado. No caso do Magazine Luiza, a URL deve ser obitda navegando para o site, clicando em **Todos os Departamentos** na parte superior esquerda, e escolhendo um dos links destacados pela área vermelha:

<img src="https://i.imgur.com/ief76w1.png" alt="" width="50%" height="50%" />

Na próxima página clique em um dos links destacados novamente pela área vermelha:

<img src="https://i.imgur.com/FNuQU2P.png" alt="" width="50%" height="50%" />

A URL da págna que abrirá é a que deve ser utilizada:

<img src="https://i.imgur.com/NmOzgTQ.png" alt="" width="50%" height="50%" />

O teste acima foi realizado clicando em: **Todos os Departamentos > Celulares > Smartphones** obtendo a URL **https://www.magazineluiza.com.br/smartphone/celulares-e-smartphones/s/te/tcsp/**

Após selecionada a URL, o programa deve ser compilado da seguinte maneira:

```sh
$ cd src/build/
$ cmake
$ make
```

Após a compilação, execute-o assim:
```sh
$ URL=https://www.magazineluiza.com.br/smartphone/celulares-e-smartphones/s/te/tcsp/ mpiexec -n <numero_de_processos> krawler_mpi 2> metrics.txt
```

As métricas estarão em `metrics.txt`. Caso queira, mude o nome do arquivo ao rodar mais de uma vez para não sobrescrevê-lo!

## Resultados
