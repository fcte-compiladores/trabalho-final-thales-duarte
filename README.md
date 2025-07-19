# C-Lox - Interpretador da Linguagem Lox em C

## Integrantes
- Thales Duarte Alcantara de Oliveira
- Matrícula: 221035095
- Turma: FGA0003 - COMPILADORES 1 - T02 (2025.1 - 46T45)

Este projeto foi desenvolvido como parte do trabalho prático da disciplina de Compiladores, com o objetivo de aprofundar o entendimento sobre a construção de interpretadores e a implementação de linguagens de programação. Ele foi baseado em três principais referências:

- O livro [Crafting Interpreters](https://craftinginterpreters.com/), que serviu de guia teórico e prático para a implementação da linguagem Lox.
- O repositório original de referência (https://github.com/britannio/lox), que traz uma implementação básica da linguagem Lox em C.
- O repositório [lox-fun-es-Thales-Duarte](https://github.com/ThalesD/lox-fun-es-Thales-Duarte), que apresenta uma versão em Python, exemplos e exercícios, permitindo estudo comparativo entre abordagens e linguagens.

## Algumas Melhorias e Diferenciais

Durante o desenvolvimento, foram realizadas diversas melhorias em relação às referências originais:

- **Testes automatizados:** Foi criado um script batch para rodar todos os exemplos essenciais automaticamente, facilitando a verificação da robustez do interpretador após cada alteração.
- **Suporte a listas:** Implementação completa de estruturas de dados do tipo lista com funções nativas para manipulação.
- **Suporte a dicionários:** Implementação completa de estruturas de dados do tipo dicionário com funções nativas para manipulação.
- **Suporte a enums:** Implementação completa de tipos enumerados com funções nativas para manipulação.
- **Suporte a funções anônimas (lambdas):** Permite criar funções sem nome, inclusive com closures e corpo de expressão ou bloco.
- **Operadores personalizados:** Permite que classes definam comportamentos personalizados para operadores aritméticos e de comparação.
- **Ferramentas de Debug**.

Essas melhorias tornam o projeto uma base sólida para estudos, experimentos e comparações no contexto da disciplina de Compiladores.

## O que é Lox?
Lox é uma linguagem de programação criada para fins educacionais, ideal para aprender sobre construção de interpretadores e compiladores.


## Arquitetura da Máquina Virtual

O C-Lox implementa uma **máquina virtual baseada em stack**, seguindo os princípios clássicos de design de linguagens de programação.

### Componentes Principais

- **Bytecode**: O código fonte Lox é compilado para um bytecode customizado com operações como `OP_CONSTANT`, `OP_ADD`, `OP_CALL`, `OP_JUMP`, etc.
- **Stack Machine**: Utiliza uma pilha para executar operações, onde valores são empilhados e desempilhados conforme necessário
- **Call Frames**: Cada função em execução possui seu próprio frame com closure, instruction pointer e slots de variáveis
- **Memory Management**: Inclui garbage collection automático para gerenciamento de memória

### Fluxo de Execução

1. **Compilação**: Código fonte Lox → Bytecode
2. **Execução**: VM executa bytecode instrução por instrução
3. **Stack Operations**: Operações realizadas na pilha (push/pop)
4. **Runtime**: Gerenciamento de memória e funções nativas


## Funcionalidades Implementadas

### Funcionalidades Básicas
- **Variáveis:** Declaração e atribuição de variáveis globais e locais
- **Funções:** Definição, chamada e recursão
- **Classes:** Declaração, instanciação e métodos
- **Closures:** Funções que capturam variáveis do escopo externo
- **Controle de fluxo:** `if`, `while`, `for`
- **Operadores:** Aritméticos, lógicos e de comparação
- **Tipos de dados:** Números, strings, booleanos, nil
- **Coletor de lixo:** Gerenciamento automático de memória

## Estrutura do Projeto
- **src/**: Código-fonte em C do interpretador.
- **examples/**: Exemplos de programas Lox para testar funcionalidades.
- **build.bat**: Script para compilar o projeto no Windows.
- **test_examples.bat**: Script para executar testes automatizados.

## Como compilar

Pré-requisito: GCC instalado no Windows.

Abra o terminal na raiz do projeto e execute:

```sh
.\build.bat

```


O executável `c-lox.exe` será gerado na raiz do projeto.

Para testar todos as funcionalidades:


```sh
.\test_examples.bat

```

## Como rodar

### Executar um arquivo Lox
```sh
.\c-lox.exe caminho\para\arquivo.lox
```
Esse comando executa o arquivo Lox especificado, rodando todo o código presente nele e exibindo a saída no terminal.

### Modo interativo (REPL)
```sh
.\c-lox.exe
```
Ao executar apenas `.\c-lox.exe` sem argumentos, o interpretador entra no modo interativo, também conhecido como REPL (Read-Eval-Print Loop). Nesse modo, você pode digitar comandos Lox linha a linha diretamente no terminal, e o resultado de cada comando é exibido imediatamente. É ideal para testar pequenos trechos de código, aprender a linguagem ou depurar comportamentos.

**Exemplo de sessão REPL:**
```
> print 1 + 2;
3
> var nome = "Lox";
> print nome;
Lox
```
Para sair do modo interativo, basta pressionar Ctrl+C ou Ctrl+D (dependendo do terminal).

### Resumo dos modos de execução
- `.\c-lox.exe` — Inicia o modo interativo (REPL)
- `.\c-lox.exe caminho\para\arquivo.lox` — Executa um arquivo Lox
- `.\c-lox.exe --ast caminho\para\arquivo.lox` — Mostra a árvore sintática (AST) do arquivo, sem executar o código

## Exemplos 
Os exemplos abaixo cobrem as principais funcionalidades trabalhadas no trabalho:

- **print/lists_test.lox** — Teste completo de listas e saída básica.
- **print/dict_test.lox** — Teste completo de dicionários.
- **print/enum_test.lox** — Teste completo de enums.
- **print/lambda_test.lox** — Teste completo de funções anônimas (lambdas).
- **class/custom_operators.lox** — Teste completo de operadores personalizados.
- **function/recursion.lox** — Funções e recursão.
- **if/if.lox** — Controle de fluxo condicional.
- **while/simple.lox** — Laços de repetição.
- **variable/global.lox** — Declaração e uso de variáveis globais.
- **operator/add.lox** — Operações aritméticas.
- **class/empty.lox** — Declaração de classes.
- **closure/nested_closure.lox** — Closures (funções que capturam variáveis do escopo externo).
- **return/in_function.lox** — Retorno de valores em funções.
- **string/literals.lox** — Manipulação de strings.
- **bool/equality.lox** — Booleanos e operações de igualdade.
- **nil/literal.lox** — Valor especial `nil` (nulo).

Para rodar um exemplo:
```sh
.\c-lox.exe examples\<caminho_do_exemplo>
```
Exemplo:
```sh
.\c-lox.exe examples\class\custom_operators.lox
```


## Testes Automatizados

Para rodar todos os exemplos essenciais automaticamente e verificar se o interpretador está funcionando corretamente, execute o script:

```
test_examples.bat
```

O script executa cada exemplo essencial e mostra a saída no terminal. Se algum teste falhar, será exibida uma mensagem de erro.

## Dicas e Solução de Problemas
- **Erro de compilação:** Verifique se o GCC está instalado e atualizado.
- **Executável não encontrado:** Certifique-se de que a compilação foi bem-sucedida e que o arquivo `c-lox.exe` está na raiz do projeto.
- **Erro ao rodar exemplos:** Verifique o caminho do arquivo e se ele existe em `examples/`.
- **Problemas com listas:** Certifique-se de que está usando as funções nativas corretas (`list()`, `append()`, `get()`, `set()`, `length()`).
- **Problemas com dicionários:** Certifique-se de que está usando as funções nativas corretas (`dict()`, `dictSet()`, `dictGet()`, `dictDelete()`, `dictLength()`).
- **Problemas com enums:** Certifique-se de que está usando as funções nativas corretas (`enum()`, `enumAddValue()`, `enumGetValue()`, `enumLength()`).




## Visualizador de AST (Árvore de Sintaxe Abstrata)

O projeto possui um modo de debug que permite visualizar a Árvore de Sintaxe Abstrata (AST) das expressões de um programa Lox, sem executar o código. Isso é útil para entender como o parser interpreta cada expressão.

### Como usar

Basta rodar o interpretador com a opção `--ast` ou `-a` antes do caminho do arquivo:

```
```

## Bugs, Limitações e Problemas Conhecidos

- O interpretador foi extensivamente testado, mas podem existir casos extremos ou de uso não previstos que não foram cobertos pelos exemplos.
- O suporte a listas, dicionários e enums é funcional, mas pode ser expandido com mais métodos nativos e integração com outras partes da linguagem.
- O coletor de lixo pode ser otimizado para cenários de uso intensivo de memória.
- Não há suporte a Unicode completo em strings.
- Melhorias incrementais podem ser feitas na performance da VM e na cobertura de testes automatizados.

# toString em instâncias

No C-Lox, o método `toString()` **é chamado automaticamente** ao imprimir uma instância com `print obj;` ou ao concatenar objetos com strings. O resultado será o valor retornado por `toString()` se ele existir na classe. Caso contrário, será exibido `NomeDaClasse instance`.
