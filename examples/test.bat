@echo off
pushd "%~dp0"

call :compile address
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile aliases
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile andor
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile any_type_as
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile any_type_info
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile any_type_inventory
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile any_type_kind
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile any_type_list
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile array_access
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile as
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile assignment
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile at
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile bitwise
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile break
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile break_to
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile cast
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile character_literals
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile complement
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile conditionless_break_label
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile constants
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile continue
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile continue_to
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile defer
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile deprecated
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile dereference
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile each_in
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile enums
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile external
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile fixed_array
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile fixed_array_assign
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile funcptr
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile functions
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile globals
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile hello_world
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile hexadecimal
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile if
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile ifelse
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile import
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile inline_declaration
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile int_ptr_cast
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile management_assign
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile management_defer
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile management_math
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile management_pass
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile mathassign
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile member
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile methods
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile multiple_declaration
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile native_linking
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile negate
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile new_cstring
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile new_delete
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile new_dynamic
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile not
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile null
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile null_checks --null-checks
if %errorlevel% neq 0 popd & exit /b %errorlevel%
null_checks\main.exe
if %errorlevel% neq 1 popd & exit /b %errorlevel%
call :compile order
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile package
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile package_use
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile pragma
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile primitives
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile repeat
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile repeat_args
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile repeat_fields
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile return_ten
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile scoped_variables
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile sizeof
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile standard
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile static_arrays
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile static_structs
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile stdcall
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile string
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile structs
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile successful
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile terminate_join
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile truefalse
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile undef
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile unless
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile unlesselse
if %errorlevel% neq 0 popd & exit /b %errorlevel%
REM This file should always fail
call :compile unsupported
if %errorlevel% neq 1 popd & exit /b %errorlevel%
call :compile until
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile until_break
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile varargs
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile variables
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile while
if %errorlevel% neq 0 popd & exit /b %errorlevel%
call :compile while_continue
if %errorlevel% neq 0 popd & exit /b %errorlevel%

:: Delete debugging dump files if present
if exist ast.txt   del /F ast.txt
if exist infer.txt del /F infer.txt
if exist ir.txt    del /F ir.txt

echo All tests compiled successfully!
popd
exit /b 0

:compile
echo ^=^> Compiling Test Program^: ^'%~1^'
adept_debug %~1/main.adept %~2
exit /b %errorlevel%
GOTO:EOF
