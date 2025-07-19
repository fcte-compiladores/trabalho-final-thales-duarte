@echo off
echo Compilando Clox...

gcc src/chunk.c src/compiler.c src/context.c src/debug.c src/errors.c src/memory.c src/object.c src/scanner.c src/semantic.c src/table.c src/type_checking.c src/value.c src/vm.c src/coverage.c src/main.c -O3 -o c-lox.exe

if %ERRORLEVEL% EQU 0 (
    echo Compilacao concluida com sucesso!
    echo Executavel: c-lox.exe
    echo.
    echo.
    echo Uso:
    echo   c-lox.exe [arquivo.lox]     - Executar arquivo
    echo   c-lox.exe --test            - Executar testes
    echo   c-lox.exe --optimize arquivo.lox - Executar com otimizacoes
    echo   c-lox.exe --ast arquivo.lox - Modo debug AST
) else (
    echo Erro na compilacao!
)

pause 