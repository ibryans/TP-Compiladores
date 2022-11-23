
section .data		; Seção de dados
M:			; Rótulo que marca o início da seção de dados
resb 10000h			; Reserva de temporários
section .text		; Seção de código
global _start		; Ponto inicial do programa
_start:			; Início do programa

section .data 		; seção de dados
	resd 1 		; reserva 4 bytes

section .text 		; voltando p/ seção de código
	mov eax,1 		; carrega a constante em eax
	mov [qword M+0x10000],eax 		; id recebe o valor da constante

section .data 		; seção de dados
	resb 100h 		; reserva 256 bytes

section .text 		; voltando p/ seção de código

section .data 		; seção de dados
	dd 10 		; reserva 4 bytes e escreve a constante

section .text 		; voltando p/ seção de código

section .data 		; seção de dados
	db "Digite seu nome: ", 0 		; reserva tamanho da string + indicador de fim da string

section .text 		; voltando p/ seção de código
	mov rcx,0 		; zera contador
	mov rsi, qword M+0x10108		; rsi recebe endereco de Exp
	mov rdi, qword M+0x0		; rdi recebe endereco de novo_temp
rotulo1: 		; next_char: rotulo p/ ler proximo char da str
	mov al, [rsi] 		; guarda o char atual da string
	mov [rdi], al 		; salva esse char no temp
	add rcx, 1 		; contador++
	add rsi, 1 		; proximo char (Exp)
	add rdi, 1 		; proximo char (temp)
	cmp al, 0 		; checa se eh fim da str
	jne rotulo1		; se não eh o fim, continua o loop (jump p/ next_char)
	sub rcx, 1 		; contador-- (remove o fim da str)

;executa interrupção de saída

	mov rsi, M+0x0		; inicio da string
	mov rdx, rcx		; tamanho da str
	mov rax, 1		; chamada para saida
	mov rdi, 1		; saida para tela
	syscall 		; chama o kernel
	mov rsi, M+0x0		; endereco do buffer
	mov rdx, 100h 		; tamanho do buffer
	mov rax, 0 		; chamada para leitura
	mov rdi, 0 		; leitura do teclado
	syscall 		
	mov rsi, qword M+0x0		; rsi recebe endereco do novo_temp
	mov rdi, qword M+0x10004		; rdi recebe endereco de id
rotulo2: 		; next_char: rotulo p/ ler proximo char da str
	mov al, [rsi] 		; guarda o char atual da string lida
	mov [rdi], al 		; salva esse char no id
	add rsi, 1 		; proximo char (string lida)
	add rdi, 1 		; proximo char (id)
	cmp al, 0Ah 		; checa pelo 'Enter' (quebra de linha = fim da string)
	jne rotulo2		; se não eh o fim, continua o loop (jump p/ next_char)
	sub rdi, 1 		; retira ultimo byte lido da str (quebra de linha)
	mov bl, 0 		; carrega delimitador de fim de string (0) em bl
	mov [rdi], bl 		; carrega bl no fim da string (troca quebra de linha pelo 0)

section .data 		; seção de dados
	resd 1 		; reserva 4 bytes

section .text 		; voltando p/ seção de código
rotulo3: 		; rot_inicio
	mov eax, [qword M+0x10000]		; carrega [ExpS.end] em eax
	mov ebx, [qword M+0x10104]		; carrega [ExpS1.end] em ebx
	cmp eax,ebx		; faz comparacao (ExpS e ExpS1)
	jle rotulo5		; pula p/ rot_verdadeiro
	mov eax,0 		; eax:=false
	jmp rotulo6 		; pula p/ rot_fim
rotulo5: 		; rot_verdadeiro
	mov eax,1 		; eax:=true
rotulo6: 		; rot_fim
	mov [qword M+0x0], eax		; guarda resultado
	mov eax, [qword M+0x0]		; carrega [Exp.end] em eax
	cmp eax,1 		; faz o teste
	jne rotulo4		; se teste deu falso, jump p/ rot_fim
	mov eax, [qword M+0x10000]		; carrega [Exp.end] em eax
	cvtsi2ss xmm0, eax 		; converte p/ real e guarda em xmm0
	movss [qword M+0x0],xmm0 		; guarda valor no novo endereco
	mov eax, [qword M+0x10104]		; carrega [F1.end] em eax
	cvtsi2ss xmm1, eax		; converte p/ float e armazena em xmm1
	movss xmm0, [qword M+0x0]		; carrega F.end em xmm0
	divss xmm0, xmm1		; faz a divisao
	movss [qword M+0x4],xmm0 		; copia resultado p/ [F.end]
	movss xmm0, [qword M+0x4] 		; carrega [Exp.end] em xmm0
	movss [qword M+0x1011a],xmm0 		; carrega id.end com Exp.end
	movss xmm0, [qword M+0x1011a] 		; real a ser convertido
	mov rsi, M+0x0		;end. temporário
	mov rcx, 0 		;contador pilha
	mov rdi, 6 		;precisao 6 casas compart
	mov rbx, 10 		;divisor
	cvtsi2ss xmm2, rbx 		;divisor real
	subss xmm1, xmm1 		;zera registrador
	comiss xmm0, xmm1 		;verifica sinal
	jae rotulo7		;salta se número positivo
	mov dl, '-' 		;senão, escreve sinal –
	mov [rsi], dl 		
	mov rdx, -1 		; rdx := -1
	cvtsi2ss xmm1, rdx 		;Converte para real
	mulss xmm0, xmm1 		;Toma módulo
	add rsi, 1 		;incrementa índice
rotulo7: 		
	roundss xmm1, xmm0, 0b0011 		;parte inteira xmm1
	subss xmm0, xmm1 		;parte frac xmm0
	cvtss2si rax, xmm1 		;convertido para int
;converte parte inteira que está em rax
rotulo8: 		
	add rcx, 1 		;incrementa contador
	cdq 		;estende edx:eax p/ div.
	idiv ebx 		;divide edx:eax por ebx
	push dx 		;empilha valor do resto
	cmp eax, 0 		;verifica se quoc. é 0
	jne rotulo8		;se não é 0, continua
	sub rdi, rcx 		;decrementa precisao
;agora, desemp valores e escreve parte int
rotulo9: 		
	pop dx 		;desempilha valor
	add dl, '0' 		;transforma em caractere
	mov [rsi], dl 		;escreve caractere
	add rsi, 1 		;incrementa base
	sub rcx, 1 		;decrementa contador
	cmp rcx, 0 		;verifica pilha vazia
	jne rotulo9		;se não pilha vazia, loop
	mov dl, '.' 		;escreve ponto decimal
	mov [rsi], dl 		;escreve ponto decimal
	add rsi, 1 		;incrementa base
;converte parte fracionaria que está em xmm0
rotulo10: 		
	cmp rdi, 0 		;verifica precisao
	jle rotulo11 		; terminou precisao ?
	mulss xmm0,xmm2 		;desloca para esquerda
	roundss xmm1,xmm0,0b0011 		;parte inteira xmm1
	subss xmm0,xmm1 		;atualiza xmm0
	cvtss2si rdx, xmm1 		;convertido para int
	add dl, '0' 		;transforma em caractere
	mov [rsi], dl 		;escreve caractere
	add rsi, 1 		;incrementa base
	sub rdi, 1 		;decrementa precisao
	jmp rotulo10		;loop
; impressão
rotulo11: 		
	mov rdx, rsi 		;calc tam str convertido
	mov rbx, M+0x0		;end. string ou temp
	sub rdx, rbx 		;tam=rsi-M-buffer.end

;executa interrupção de saída

	mov rsi, M+0x0		; inicio da string
			; tamanho da string ja está em rdx
	mov rax, 1		; chamada para saida
	mov rdi, 1		; saida para tela
	syscall 		; chama o kernel

section .data 		; seção de dados
	db ": Ola' ", 0 		; reserva tamanho da string + indicador de fim da string

section .text 		; voltando p/ seção de código
	mov rcx,0 		; zera contador
	mov rsi, qword M+0x1011e		; rsi recebe endereco de Exp
	mov rdi, qword M+0x0		; rdi recebe endereco de novo_temp
rotulo12: 		; next_char: rotulo p/ ler proximo char da str
	mov al, [rsi] 		; guarda o char atual da string
	mov [rdi], al 		; salva esse char no temp
	add rcx, 1 		; contador++
	add rsi, 1 		; proximo char (Exp)
	add rdi, 1 		; proximo char (temp)
	cmp al, 0 		; checa se eh fim da str
	jne rotulo12		; se não eh o fim, continua o loop (jump p/ next_char)
	sub rcx, 1 		; contador-- (remove o fim da str)

;executa interrupção de saída

	mov rsi, M+0x0		; inicio da string
	mov rdx, rcx		; tamanho da str
	mov rax, 1		; chamada para saida
	mov rdi, 1		; saida para tela
	syscall 		; chama o kernel
	mov rcx,0 		; zera contador
	mov rsi, qword M+0x10004		; rsi recebe endereco de Exp
	mov rdi, qword M+0x0		; rdi recebe endereco de novo_temp
rotulo13: 		; next_char: rotulo p/ ler proximo char da str
	mov al, [rsi] 		; guarda o char atual da string
	mov [rdi], al 		; salva esse char no temp
	add rcx, 1 		; contador++
	add rsi, 1 		; proximo char (Exp)
	add rdi, 1 		; proximo char (temp)
	cmp al, 0 		; checa se eh fim da str
	jne rotulo13		; se não eh o fim, continua o loop (jump p/ next_char)
	sub rcx, 1 		; contador-- (remove o fim da str)
	mov rsi, M+0x0		; inicio da string
	add rsi, rcx 		; soma contador ao inicio
	mov al, 0xA 		; reg. al recebe quebra de linha
	mov [rsi], al 		; quebra de linha no final do char
	add rcx, 1 		; contador++ p/ n dar pau com a quebra de linha

;executa interrupção de saída

	mov rsi, M+0x0		; inicio da string
	mov rdx, rcx		; tamanho da str
	mov rax, 1		; chamada para saida
	mov rdi, 1		; saida para tela
	syscall 		; chama o kernel
	mov eax,1		; carrega a constante inteira em eax
	mov [qword M+0x0], eax 		; o novo endereco recebe o valor da constante
	mov eax, [qword M+0x10000]		; carrega [T.end] em eax
	mov ebx, [qword M+0x0]		; carrega [T1.end] em ebx
	add eax,ebx		; faz adicao
	mov [qword M+0x4],eax 		; copia resultado p/ novo endereco
	mov eax, [qword M+0x4] 		; carrega [Exp.end] em eax
	mov [qword M+0x10000],eax 		; carrega id.end com Exp.end
	jmp rotulo3		; jump p/ rot_inicio
rotulo4: 		; rot_fim

; Halt
mov rax, 60 		; Chamada de saída
mov rdi, 0 			; Código de saída sem erros
syscall 			; Chama o kernel
