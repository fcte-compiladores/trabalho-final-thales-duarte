@echo off
echo ===========================
echo Testando exemplos essenciais
echo ===========================
echo.

echo --- Executando: examples\print\lists_test.lox ---
.\c-lox.exe examples\print\lists_test.lox

echo.
echo --- Executando: examples\print\dict_test.lox ---
.\c-lox.exe examples\print\dict_test.lox

echo.
echo --- Executando: examples\print\enum_test.lox ---
.\c-lox.exe examples\print\enum_test.lox

echo.
echo --- Executando: examples\print\lambda_test.lox ---
.\c-lox.exe examples\print\lambda_test.lox

echo.
echo --- Executando: examples\class\custom_operators.lox ---
.\c-lox.exe examples\class\custom_operators.lox

echo.
echo --- Executando: examples\function\recursion.lox ---
.\c-lox.exe examples\function\recursion.lox

echo.
echo --- Executando: examples\if\if.lox ---
.\c-lox.exe examples\if\if.lox

echo.
echo --- Executando: examples\while\simple.lox ---
.\c-lox.exe examples\while\simple.lox

echo.
echo --- Executando: examples\variable\global.lox ---
.\c-lox.exe examples\variable\global.lox

echo.
echo --- Executando: examples\operator\add.lox ---
.\c-lox.exe examples\operator\add.lox

echo.
echo --- Executando: examples\class\empty.lox ---
.\c-lox.exe examples\class\empty.lox

echo.
echo --- Executando: examples\closure\nested_closure.lox ---
.\c-lox.exe examples\closure\nested_closure.lox

echo.
echo --- Executando: examples\return\in_function.lox ---
.\c-lox.exe examples\return\in_function.lox

echo.
echo --- Executando: examples\string\literals.lox ---
.\c-lox.exe examples\string\literals.lox

echo.
echo --- Executando: examples\bool\equality.lox ---
.\c-lox.exe examples\bool\equality.lox

echo.
echo --- Executando: examples\nil\literal.lox ---
.\c-lox.exe examples\nil\literal.lox

echo ===========================
echo Testes finalizados
echo ===========================
pause 