#include <msp430.h>
#include "grlib.h"
#include "uart_STDIO.h"
#include "Crystalfontz128x128_ST7735.h"
#include "HAL_MSP430G2_Crystalfontz128x128_ST7735.h"
#include <stdio.h>
#include <stdlib.h>



char aciertos_text[20];
char cont;



//FUNCIONES DEL RELOJ

void Set_Clk(char VEL){
    BCSCTL2 = SELM_0 | DIVM_0 | DIVS_0;
    switch(VEL){
    case 1:
        if (CALBC1_1MHZ != 0xFF) {
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
            DCOCTL = CALDCO_1MHZ;
        }
        break;
    case 8:

        if (CALBC1_8MHZ != 0xFF) {
            __delay_cycles(100000);
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_8MHZ;      /* Set DCO to 8MHz */
            DCOCTL = CALDCO_8MHZ;
        }
        break;
    case 12:
        if (CALBC1_12MHZ != 0xFF) {
            __delay_cycles(100000);
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_12MHZ;     /* Set DCO to 12MHz */
            DCOCTL = CALDCO_12MHZ;
        }
        break;
    case 16:
        if (CALBC1_16MHZ != 0xFF) {
            __delay_cycles(100000);
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_16MHZ;     /* Set DCO to 16MHz */
            DCOCTL = CALDCO_16MHZ;
        }
        break;
    default:
        if (CALBC1_1MHZ != 0xFF) {
            DCOCTL = 0x00;
            BCSCTL1 = CALBC1_1MHZ;      /* Set DCO to 1MHz */
            DCOCTL = CALDCO_1MHZ;
        }
        break;

    }
    BCSCTL1 |= XT2OFF | DIVA_0;
    BCSCTL3 = XT2S_0 | LFXT1S_2 | XCAP_1;
}




//DECLARO MOMENT
int moment;

//FUNCION RANDOM
int get_rand(int numero_opciones)
{
    return moment % numero_opciones;
}


//DOS FUNCIONES PARA LEER Y DAR VALOR DEL POTENCIOMETRO

void inicia_ADC(char canales)
{
    ADC10CTL0 &= ~ENC;      //deshabilita ADC
    ADC10CTL0 = ADC10ON | ADC10SHT_3 | SREF_0|ADC10IE; //enciende ADC, S/H lento, REF:VCC, con INT
    ADC10CTL1 = CONSEQ_0 | ADC10SSEL_0 | ADC10DIV_0 | SHS_0 | INCH_0;
    //Modo simple, reloj ADC, sin subdivision, Disparo soft, Canal 0
    ADC10AE0 = canales; //habilita los canales indicados
    ADC10CTL0 |= ENC; //Habilita el ADC
}

int lee_ch(char canal){ //SI llamamos a lee_ch(0) leo el ejeX y si quiero ver el eje Y con lee_ch(3)
    ADC10CTL0 &= ~ENC;                  //deshabilita el ADC
    ADC10CTL1&=(0x0fff);                //Borra canal anterior
    ADC10CTL1|=canal<<12;               //selecciona nuevo canal
    ADC10CTL0|= ENC;                    //Habilita el ADC
    ADC10CTL0|=ADC10SC;                 //Empieza la conversi�n
    LPM0;                               //Espera fin en modo LPM0
    return(ADC10MEM);                   //Devuelve valor leido
    }

//DECLARAMOS EL CONTEXTO PARA PODER PINTAR BIEN

Graphics_Context g_sContext;

//DECLARAMOS ALGUNAS VARIABLES AUXILIARES
int t;
int i;



//PRIMERO BORRAMOS TODA LA INFORMACION DE LA FLASH AL INICIAR EL PROGRAMA

void borrar_flash()
{
    unsigned char *puntero;

    FCTL3 = FWKEY;            // Accedemos a la memoria flash
    FCTL1 = FWKEY + ERASE;    // Ponemos la opcion de borrado
    for (puntero = (unsigned char *)0x1000; puntero <= (unsigned char *)0x1040; puntero++)
    {
        *puntero = 0;       // Recorremos cada posicion y la ponemos a cero con el puntero
    }
    FCTL1 = FWKEY;            // Quitamos el modo de borrado
    FCTL3 = FWKEY + LOCK;     // Bloqueamos la memoria flash
}

//DESPUES ESCRIBIMOS LA INFORMACION RECIBIDA EN LA FLASH
void guarda_flash(char *data, unsigned int length)
{
    unsigned char *puntero;

    FCTL3 = FWKEY;             // Desbloquear la memoria flash
    FCTL1 = FWKEY + WRT;       // Habilitar escritura
    puntero = (unsigned char *)0x1000;
    for (i=0; i<length; i++)
    {
        *puntero++ = data[i];  // Escribir datos en la memoria flash
    }
    FCTL1 = FWKEY;             // Detener la escritura
    FCTL3 = FWKEY + LOCK;      // Bloquear la memoria flash
}


//FUNCIONES REFERIDAS AL ZUMBADOR


//DECLARAMOS ALGUNAS NOTAS QUE VAMOS A UTILIZAR
#define     SOL 20408
#define     RE  27240
#define     SIb 17168
#define     DO  30578
#define     MI  24270



void inicializa_pwm()
{
    P2DIR |= BIT6;          // P2.6 como salida
    P2SEL |= BIT6;          // P2.6 funci�n PWM
    P2SEL2 &= ~BIT6;        // P2.6 GPIO secundario

    TA0CTL = TASSEL_2 | MC_1;    // Fuente SMCLK, modo Up
    TA0CCTL1 = OUTMOD_7;         // Modo PWM Reset/Set
    TA0CCR0 = 1000;              // Inicialmente frecuencia base
    TA0CCR1 = 500;               // Inicialmente ciclo 50%
}

void toca_nota(unsigned int nota)
{
    TA0CCR0=nota;
    TA0CCR1=nota>>1;
}

void apagar_zumbador()
{
    // Apagar el sonido (PWM)
    TA0CCR0 = 0;
    TA0CCR1 = 0;
}


void melodia()
{
    cont = 0; // Reiniciar contador antes de comenzar
    while (cont < 150)
    {
        if (cont < 50)
        {
            toca_nota(SOL);
        }
        else if (cont < 100)
        {
            toca_nota(RE);
        }
        else if (cont < 150)
        {
            toca_nota(SOL);
        }
        __delay_cycles(100000); // Esperar un tiempo para que se perciba la nota
        cont++;
    }
    // Apagar el zumbador al terminar
    apagar_zumbador();

}

void melodia_bien()
{
    cont = 0; // Reiniciar contador antes de comenzar
    while (cont < 30)
    {
        if (cont < 30)
        {
            toca_nota(SIb);
        }
        __delay_cycles(100000); // Esperar un tiempo para que se perciba la nota
        cont++;
    }
    // Apagar el zumbador al terminar
    TA0CCR0 = 0;             // Apagar PWM
    TA0CCR1 = 0;
  //  P2SEL &= ~BIT6;          // Desactivar funci�n PWM del pin
  //  P2OUT &= ~BIT6;          // Asegurar que el pin est� apagado
}

void melodia_moneda()
{
    cont = 0; // Reiniciar el contador antes de comenzar
    while (cont < 150) // 24 pasos para la melod�a inicial
    {
        if (cont < 30) // Mi, Mi, Mi
        {
            toca_nota(MI);
        }
        else if (cont < 40) // Pausa
        {
            apagar_zumbador();
        }
        else if (cont < 50) // Do
        {
            toca_nota(DO);
        }
        else if (cont < 60) // Mi
        {
            toca_nota(MI);
        }
        else if (cont < 90) // Sol (sostenido m�s tiempo)
        {
            toca_nota(SOL);
        }
        else if (cont < 100) // Pausa
        {
            apagar_zumbador();
        }
        else if (cont < 120) // Sol
        {
            toca_nota(SOL);
        }
        else if (cont < 130) // Do
        {
            toca_nota(DO);
        }

        // Espera entre notas
        __delay_cycles(100000); // Ajusta este valor para controlar la duraci�n de cada nota
        cont++;
    }

    // Apagar el zumbador al terminar
    apagar_zumbador();
}


//PARA EL JUEGO 1

// Definimos la estructura de los enemigos y las constantes
#define MOVIMIENTO_ENEMIGO 2 // Velocidad de movimiento de los enemigos

// Estructura para representar a los enemigos
typedef struct {
    int eje_x;         // Posición en el eje X
    int eje_y;         // Posición en el eje Y
    int directionX;    // Dirección de movimiento en X (-1, 0, 1)
    int directionY;    // Dirección de movimiento en Y (-1, 0, 1)
} Enemigo;

// Declaramos el array de enemigos
Enemigo enemigo[4];
int enemigo_activo = -1; // Inicialmente ningún enemigo está activo

//POSICIONES DE LOS ENEMIGOS DEL PRIMER JUEGO
Graphics_Rectangle enemigo_up = {55,0,69,13};
Graphics_Rectangle enemigo_left = {0,55,13,69};
Graphics_Rectangle enemigo_down = {55,113,69,127};
Graphics_Rectangle enemigo_right = {113,55,127,69};


//DECLARAMOS LA VARIABLE DEL NOMBRE QUE USAREMOS PARA LEER POR PUERTO SERIE
char nombre[4];

//DECLARAMOS LA VARIABLE ESTADO PARA PASAR DE CASE
char Estado;


main()
{

//DECLARAMOS TODAS LAS VARIABLES QUE NECESITAMOS EN EL MAIN
char cont_kills;
enum estados{pantalla_inicio,meter_nombre,Nivel_1,explicacion_1,explicacion_2,explicacion_3,intermedio_1_1,intermedio_1_2,intermedio_1_3,Juego_1,Nivel_2,Juego_2,Nivel_3,Juego_3,intermedio_1,intermedio_2,intermedio_4,intermedio_3,espera, pasa_nivel_1, pasa_nivel_2};
Estado  = pantalla_inicio;

char punt = 0;
char figura_random;
char j = 0;


static int tiempo_flecha = 100;
static int vel_caida_manzana = 10;

char cuadros_movimiento = 0;

inicializa_pwm();

//SITUAMOS EL JOYSTICK EN MEDIO PARA COMPARAR
int ejey = 512;
int ejex = 512;

//LLAMAMOS A LA FUNCION
inicia_ADC(BIT0 + BIT3);

WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
Set_Clk(16);

//LINEA DE CODIGO NECESARIA PARA EL TIEMPO DE ESPERA DE LA MEMORIA FLASH
FCTL2 = FWKEY + FSSEL_2 + 34;

// Timer 20 ms
TA1CCTL0=CCIE; //CCIE=1
TA1CTL=TASSEL_2| ID_3 | MC_1; //ACLK, DIV=1, UP
TA1CCR0=19999; //periodo

//TIMER ZUMBADORCITO
/*TA0CCTL0=CCIE; //CCIE=1
TA0CTL=TASSEL_1| MC_1; //ACLK, DIV=1, UP
*/
UARTinit(16);

/*------ Pines de E/S involucrados:------------------------*/
    P1SEL2 = BIT1 | BIT2;  //P1.1 RX, P1.2: TX
    P1SEL = BIT1 | BIT2;
    P1DIR = BIT0 + BIT2;
    P1REN = BIT3;             //Boton en P1.3

    //Declaro el boton del joystick
        P2DIR &= ~BIT5;
        P2OUT |= BIT5;

    //BOTON DE LA PLACA
        P1DIR &= ~BIT1;  // Configura P1.1 como entrada (baja)
        P1REN |= BIT1;    // Habilita resistencia de pull-up/pull-down
        P1OUT |= BIT1;    // Configura la resistencia�como�pull-up

Graphics_Context g_sContext;

/* Set default screen orientation */
Crystalfontz128x128_Init(); //Arrancar la pantalla
Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);//Arrancar la pantalla
Graphics_initContext(&g_sContext, &g_sCrystalfontz128x128);//Inicio g_sContex con la libreia g_sCrystalfontz128x128

/* Initializes graphics context */
Graphics_setFont(&g_sContext, &g_sFontCmss14b);

//DEFINIMOS EL TAMA�O DEL JUGADOR
Graphics_Rectangle jugador = {60,60,68,68};
Graphics_Rectangle pantalla_negra = {0,0,127,127};
//POSICIONES DE LA HOJA
Graphics_Rectangle pist_up = {62,56,66,59};
Graphics_Rectangle pist_left = {56,62,59,66};
Graphics_Rectangle pist_down = {62,69,66,72};
Graphics_Rectangle pist_right = {69,62,72,66};
//FILO
Graphics_Rectangle filo_up = {63,53,65,55};
Graphics_Rectangle filo_left = {53,63,55,65};
Graphics_Rectangle filo_down = {63,73,65,75};
Graphics_Rectangle filo_right = {73,63,75,65};
//RECTANGULOS JUEGO 2
Graphics_Rectangle rectangulo_vert = {58,40,68,87};
Graphics_Rectangle punta_up = {60,10,66,39};
Graphics_Rectangle punta_down = {60,88,66,107};
Graphics_Rectangle rectangulo_hor = {40,58,87,68};
Graphics_Rectangle punta_left = {10,60,39,66};
Graphics_Rectangle punta_right = {89,60,109,66};

  //PONEMOS LA PANTALLA DE INICIO DEL JUEGUITO
  Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
  Graphics_clearDisplay(&g_sContext);

  Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
  Graphics_drawString(&g_sContext,"ARCADE", 6, 35, 10, OPAQUE_TEXT);
  Graphics_drawString(&g_sContext,"GAMES", 5, 37, 30, OPAQUE_TEXT);
  Graphics_drawString(&g_sContext,"Introduce una", 20, 15, 80, OPAQUE_TEXT);
  Graphics_drawString(&g_sContext,"moneda", 6, 30, 100, OPAQUE_TEXT);

  UARTprint("DISFRUTE LA EXPERIENCIA");
  UARTprintCR("\n");


  //DECLARAMOS LOS PINES QUE VAMOS A USAR PARA EL ZUMBADOR

 /* P1REN |=BIT1+BIT2;
  P1OUT |=BIT1+BIT2;
  P1IFG=0;
  P1IE= BIT1+BIT2;
  P1IES |=BIT1+BIT2;
*/
/*
  P2DIR|=BIT6;            //P2.6 salida
  P2SEL|= BIT6;           //P2.6 pwm
  P2SEL2&=~(BIT6);        //P2.7 gpio
*/



  __bis_SR_register(GIE); // Habilita las interrupciones globalmente



while(1){
    //Bucle de bajo consumo


     LPM0;

        switch(Estado)
        {
        case pantalla_inicio:
            ejex=lee_ch(0);
            //if(ejex<150)//Muevo el joystick a la izquierda
            if(ejex>800)//Muevo el joystick a la derecha
            Estado = meter_nombre;
            break;
        case meter_nombre:


            melodia_moneda();
            //Pedimos por puerto serie el valor del nombre
            UARTprint("\n\rIntroduce tus tres iniciales");
            UARTprint("\n");
            //for (i=0; i<3; i++)


                            UARTgets(nombre,4);  // recibir caracteres del puerto serie


                            borrar_flash();
                            guarda_flash(nombre, 3);
            Estado = Nivel_1;
            break;

        case Nivel_1:
            Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_clearDisplay(&g_sContext);
            //DIBUJA NIVEL
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_drawString(&g_sContext,"NIVEL 1", 7, 40, 40, OPAQUE_TEXT);
            //DIBUJA PULSA BOTON
            //pulsa_boton();
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
               Graphics_drawString(&g_sContext,"Pulsa boton para", 16, 12, 80, OPAQUE_TEXT);
               Graphics_drawString(&g_sContext,"continuar", 9, 35, 96, OPAQUE_TEXT);
            Estado = intermedio_1;
            cont_kills=0;
           // melodia();
               break;
        case intermedio_1:
            if (j == 0) // Solo reproducir la melod�a una vez
                {
                melodia();
                    j = 1; // Marcar que la melod�a ya se ha ejecutado
                }

            if(!(P2IN&BIT5)) //PULSO EL BOTON PARA PASAR AL SIGUIENTE ESTADO
            {
                           Estado = Juego_1;
                           t = 0;
                           j = 0;
            }
               break;

        case Juego_1:
            // PINTAMOS EL FONDO
            Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_fillRectangle(&g_sContext, &pantalla_negra);

            // PINTAMOS EL CUADRADO DEL JUGADOR EN EL CENTRO
            Graphics_setForegroundColor(&g_sContext,  GRAPHICS_COLOR_AQUA);
            Graphics_drawRectangle(&g_sContext, &jugador);
            Graphics_fillRectangle(&g_sContext, &jugador);

            // CONTROL DEL JOYSTICK PARA EL CUADRADO CHICO (INDICADOR DE DIRECCIÓN)
            ejex = lee_ch(0);
            ejey = lee_ch(3);

            // Control del Joystick para las direcciones
            if (ejex < 200) // Mirando a la izquierda
            {
                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GRAY);
                Graphics_fillRectangle(&g_sContext, &pist_left);
                Graphics_fillRectangle(&g_sContext, &filo_left);
            }
            else if (ejey < 200) // Mirando hacia abajo
            {
                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GRAY);
                Graphics_fillRectangle(&g_sContext, &pist_down);
                Graphics_fillRectangle(&g_sContext, &filo_down);
            }
            else if (ejex > 700) // Mirando a la derecha
            {
                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GRAY);
                Graphics_fillRectangle(&g_sContext, &pist_right);
                Graphics_fillRectangle(&g_sContext, &filo_right);
            }
            else if (ejey > 700) // Mirando hacia arriba
            {
                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GRAY);
                Graphics_fillRectangle(&g_sContext, &pist_up);
                Graphics_fillRectangle(&g_sContext, &filo_up);
            }

            // CONTROL DE ENEMIGOS
            if (t >= 100) // Cada 0.5 segundos
            {
                t = 0; // Reiniciamos el temporizador

                // Seleccionamos un enemigo aleatorio para aparecer
                unsigned char lado = rand() % 4; // 0: derecha, 1: arriba, 2: izquierda, 3: abajo

                //unsigned char lado = get_rand(4);

                Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);

                if (lado == 0) // Desde la derecha
                {
                    enemigo_activo = 0;
                    enemigo[enemigo_activo].eje_x = 128 - 7; // Extremo derecho
                    enemigo[enemigo_activo].eje_y = 64; // Posición aleatoria en Y
                    enemigo[enemigo_activo].directionX = -3;
                    enemigo[enemigo_activo].directionY = 0;
                }
                else if (lado == 1) // Desde arriba
                {
                    enemigo_activo = 1;
                    enemigo[enemigo_activo].eje_x = 64; // Posición aleatoria en X
                    enemigo[enemigo_activo].eje_y = 0; // Extremo superior
                    enemigo[enemigo_activo].directionX = 0;
                    enemigo[enemigo_activo].directionY = 3;
                }
                else if (lado == 2) // Desde la izquierda
                {
                    enemigo_activo = 2;
                    enemigo[enemigo_activo].eje_x = 0; // Extremo izquierdo
                    enemigo[enemigo_activo].eje_y = 64; // Posición aleatoria en Y
                    enemigo[enemigo_activo].directionX = 3;
                    enemigo[enemigo_activo].directionY = 0;
                }
                else if (lado == 3) // Desde abajo
                {
                    enemigo_activo = 3;
                    enemigo[enemigo_activo].eje_x = 64; // Posición aleatoria en X
                    enemigo[enemigo_activo].eje_y = 128 - 7; // Extremo inferior
                    enemigo[enemigo_activo].directionX = 0;
                    enemigo[enemigo_activo].directionY = -3;
                }

                // Dibujamos el enemigo inicializado
                Graphics_Rectangle enemigo_rect = {enemigo[enemigo_activo].eje_x, enemigo[enemigo_activo].eje_y,
                                                   enemigo[enemigo_activo].eje_x + 7, enemigo[enemigo_activo].eje_y + 7};
                Graphics_fillRectangle(&g_sContext, &enemigo_rect);
            }

            // Movemos al enemigo activo
            if (enemigo_activo != -1)
            {
                enemigo[enemigo_activo].eje_x += enemigo[enemigo_activo].directionX * MOVIMIENTO_ENEMIGO;
                enemigo[enemigo_activo].eje_y += enemigo[enemigo_activo].directionY * MOVIMIENTO_ENEMIGO;

                // Dibujamos el enemigo en su nueva posición
                Graphics_Rectangle enemigo_rect = {enemigo[enemigo_activo].eje_x, enemigo[enemigo_activo].eje_y,
                                                   enemigo[enemigo_activo].eje_x + 7, enemigo[enemigo_activo].eje_y + 7};
                Graphics_fillRectangle(&g_sContext, &enemigo_rect);
            }

            // DETECCIÓN DE COLISIÓN ENTRE EL ENEMIGO Y LOS FILOS
            if (enemigo_activo != -1)
            {
                int colision_con_filo = 0; // Variable para controlar si hubo colisión con un filo

                // Colisión con el filo izquierdo
                if (ejex < 200 && enemigo[enemigo_activo].eje_x < filo_left.xMax && enemigo[enemigo_activo].eje_x + 10 > filo_left.xMin &&
                    enemigo[enemigo_activo].eje_y + 10 > filo_left.yMin && enemigo[enemigo_activo].eje_y < filo_left.yMax)
                {
                    colision_con_filo = 1;
                    melodia_bien();
                }
                // Colisión con el filo superior
                else if (ejey >700 && enemigo[enemigo_activo].eje_x + 10 > filo_up.xMin && enemigo[enemigo_activo].eje_x < filo_up.xMax &&
                         enemigo[enemigo_activo].eje_y < filo_up.yMax && enemigo[enemigo_activo].eje_y + 10 > filo_up.yMin)
                {
                    colision_con_filo = 1;
                    melodia_bien();
                }
                // Colisión con el filo derecho
                else if (ejex > 700 && enemigo[enemigo_activo].eje_x < filo_right.xMax && enemigo[enemigo_activo].eje_x + 10 > filo_right.xMin &&
                         enemigo[enemigo_activo].eje_y + 10 > filo_right.yMin && enemigo[enemigo_activo].eje_y < filo_right.yMax)
                {
                    colision_con_filo = 1;
                    melodia_bien();
                }
                // Colisión con el filo inferior
                else if (ejey <200 && enemigo[enemigo_activo].eje_x + 10 > filo_down.xMin && enemigo[enemigo_activo].eje_x < filo_down.xMax &&
                         enemigo[enemigo_activo].eje_y + 10 > filo_down.yMin && enemigo[enemigo_activo].eje_y < filo_down.yMax)
                {
                    melodia_bien();
                    colision_con_filo = 1;
                }

                // Si hubo colisión con algún filo visible, sumamos la kill y reiniciamos el enemigo
                if (colision_con_filo)
                {
                    cont_kills++; // Incrementamos el contador de muertes
                    enemigo_activo = -1; // Reseteamos el enemigo
                }
            }


            // Mostrar el contador de kills en la parte superior izquierda (igual que el contador de aciertos)
            char kills_text[20];
            sprintf(kills_text, "Kills: %d", cont_kills);
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_drawString(&g_sContext, kills_text, -1, 5, 5, OPAQUE_TEXT); // Usamos la misma función que en "Aciertos"

            // DIBUJAR UNA EA HORIZONTAL BLANCA (igual que el juego de aciertos)
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_drawLine(&g_sContext, 0, 30, 127, 30); // Línea en Y=30

            // CAMBIO DE NIVEL
            if (cont_kills >= 10)
            {
                Estado = Nivel_2;
            }

            /*
            // DETECC PARA CAMBIO DE ESTADO
            if (!(P1IN & BIT1))
            {
                _delay_cycles(50000);

                if (!(P1IN & BIT1))
                {
                    Estado = pasa_nivel_1;
                    t = 0;
                }
            }
            */
            break;


        case Nivel_2:
            Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_clearDisplay(&g_sContext);
            //DIBUJA NIVEL
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_drawString(&g_sContext,"NIVEL 2", 7, 40, 40, OPAQUE_TEXT);
            //DIBUJA PULSA BOTON

           // pulsa_boton();
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
               Graphics_drawString(&g_sContext,"Pulsa boton para", 16, 12, 80, OPAQUE_TEXT);
               Graphics_drawString(&g_sContext,"continuar", 9, 35, 96, OPAQUE_TEXT);
            Estado = intermedio_2;
            cont_kills=0;
           // melodia();
               break;
        case intermedio_2:
            if (j == 0) // Solo reproducir la melod�a una vez
                {
                    melodia();
                    j = 1; // Marcar que la melod�a ya se ha ejecutado
                }

               if(!(P2IN&BIT5)) //PULSO BOTON
               {
                             Estado = Juego_2;
                             t=0;
                             j=0;
               }
               break;




        case Juego_2:

            // PINTAMOS EL FONDO LO PRIMERO
            Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_clearDisplay(&g_sContext);
            char acierto_joystick = 0;
            figura_random = rand() % 8; // GENERAMOS UN NUMERO ENTRE ALEATORIO ENTRE 0 Y 7
            //figura_random = get_rand(8);
            // MOSTRAR EL CONTADOR DE PUNTOS EN LA PANTALLA
            char aciertos_text[20];
            sprintf(aciertos_text, "Aciertos: %d", punt);
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_drawString(&g_sContext, aciertos_text, -1, 5, 5, OPAQUE_TEXT);

            switch (figura_random) {
                case 0: // FLECHA ARRIBA BIEN
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
                    Graphics_fillRectangle(&g_sContext, &rectangulo_vert);
                    Graphics_fillRectangle(&g_sContext, &punta_up);
                    break;
                case 1: // FLECHA ARRIBA MAL
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
                    Graphics_fillRectangle(&g_sContext, &rectangulo_vert);
                    Graphics_fillRectangle(&g_sContext, &punta_up);
                    break;
                case 2: // FLECHA ABAJO BIEN
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
                    Graphics_fillRectangle(&g_sContext, &rectangulo_vert);
                    Graphics_fillRectangle(&g_sContext, &punta_down);
                    break;
                case 3: // FLECHA ABAJO MAL
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
                    Graphics_fillRectangle(&g_sContext, &rectangulo_vert);
                    Graphics_fillRectangle(&g_sContext, &punta_down);
                    break;
                case 4: // FLECHA A LA IZQUIERDA BIEN
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
                    Graphics_fillRectangle(&g_sContext, &rectangulo_hor);
                    Graphics_fillRectangle(&g_sContext, &punta_left);
                    break;
                case 5: // FLECHA A LA IZQUIERDA MAL
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
                    Graphics_fillRectangle(&g_sContext, &rectangulo_hor);
                    Graphics_fillRectangle(&g_sContext, &punta_left);
                    break;
                case 6: // FLECHA A LA DERECHA BIEN
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLUE);
                    Graphics_fillRectangle(&g_sContext, &rectangulo_hor);
                    Graphics_fillRectangle(&g_sContext, &punta_right);
                    break;
                case 7: // FLECHA A LA DERECHA MAL
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
                    Graphics_fillRectangle(&g_sContext, &rectangulo_hor);
                    Graphics_fillRectangle(&g_sContext, &punta_right);
                    break;
            }

            t = 0;
            while (t < tiempo_flecha) { // TIMER DE UNOS DOS SEGUNDOS DE ESPERA
                ejex = lee_ch(0); // DEBEMOS DE LEER DE NUEVO LOS EJES DENTRO DEL WHILE
                ejey = lee_ch(3);

                if (acierto_joystick == 0) { // SOLO UN PUNTO POR CICLO DE ESPERA
                    switch (figura_random) {
                        case 0: // FLECHA ARRIBA CORRECTO
                            if (ejey > 700) {
                                punt++;
                                acierto_joystick = 1;
                                melodia_bien();
                            }
                            break;
                        case 1: // FLECHA ARRIBA INCORRECTO
                            if (ejey < 200) {
                                punt++;
                                acierto_joystick = 1;
                                melodia_bien();
                            }
                            break;
                        case 2: // FLECHA ABAJO CORRECTO
                            if (ejey < 200) {
                                punt++;
                                acierto_joystick = 1;
                                melodia_bien();
                            }
                            break;
                        case 3: // FLECHA ABAJO INCORRECTO
                            if (ejey > 700) {
                                punt++;
                                acierto_joystick = 1;
                                melodia_bien();
                            }
                            break;
                        case 4: // FLECHA IZQUIERDA CORRECTO
                            if (ejex < 200) {
                                punt++;
                                acierto_joystick = 1;
                                melodia_bien();
                            }
                            break;
                        case 5: // FLECHA IZQUIERDA INCORRECTO
                            if (ejex > 700) {
                                punt++;
                                acierto_joystick = 1;
                                melodia_bien();
                            }
                            break;
                        case 6: // FLECHA DERECHA CORRECTO
                            if (ejex > 700) {
                                punt++;
                                acierto_joystick = 1;
                                melodia_bien();
                            }
                            break;
                        case 7: // FLECHA DERECHA INCORRECTO
                            if (ejex < 200) {
                                punt++;
                                acierto_joystick = 1;
                                melodia_bien();
                            }
                            break;
                    }

                    // ACTUALIZAR EL CONTADOR EN PANTALLA EN TIEMPO REAL
                    sprintf(aciertos_text, "Aciertos: %d", punt);
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                    Graphics_drawString(&g_sContext, aciertos_text, -1, 5, 5, OPAQUE_TEXT);
                }

                __delay_cycles(20000); // PEQUE�A PAUSA PARA ESTABILIZAR LA LECTURA
            }

            // LIMPIAR LA PANTALLA SEGUN LA FIGURA
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            switch (figura_random) {
                case 0:
                case 1:
                    Graphics_fillRectangle(&g_sContext, &rectangulo_vert);
                    Graphics_fillRectangle(&g_sContext, &punta_up);
                    break;
                case 2:
                case 3:
                    Graphics_fillRectangle(&g_sContext, &rectangulo_vert);
                    Graphics_fillRectangle(&g_sContext, &punta_down);
                    break;
                case 4:
                case 5:
                    Graphics_fillRectangle(&g_sContext, &rectangulo_hor);
                    Graphics_fillRectangle(&g_sContext, &punta_left);
                    break;
                case 6:
                case 7:
                    Graphics_fillRectangle(&g_sContext, &rectangulo_hor);
                    Graphics_fillRectangle(&g_sContext, &punta_right);
            }

            // VERIFICAR SI EL JUGADOR PASA AL SIGUIENTE NIVEL
            if (punt >= 10)
            {
                Estado = Nivel_3;
            }
            break;

            /*

               if(!(P1IN & BIT1))
               {
                   _delay_cycles(50000);

                   if(!(P1IN & BIT1))
                   {
                       Estado = pasa_nivel_2;
                       t=0;
                   }
               }



        case pasa_nivel_2:
            Graphics_drawString(&g_sContext,"�Seguro que quieres", 20, 12, 50, OPAQUE_TEXT);
            Graphics_drawString(&g_sContext,"pasar de juego?", 15, 35, 70, OPAQUE_TEXT);
            if(!(P1IN & BIT1))
            {
                _delay_cycles(50000);

                if(!(P1IN & BIT1))
                {
                    Estado = Nivel_3;
                    t=0;
                }
            }
            else if(!(P2IN&BIT5)) //PULSO EL BOTON PARA PASAR AL SIGUIENTE ESTADO
            {
                           Estado = Juego_2;
                           t = 0;
            }
               break;
                */
        case Nivel_3:
            Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
            Graphics_clearDisplay(&g_sContext);
            //DIBUJA NIVEL
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
            Graphics_drawString(&g_sContext,"NIVEL 3", 7, 40, 40, OPAQUE_TEXT);
            //DIBUJA PULSA BOTON
            //pulsa_boton();
            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
               Graphics_drawString(&g_sContext,"Pulsa boton para", 16, 12, 80, OPAQUE_TEXT);
               Graphics_drawString(&g_sContext,"continuar", 9, 35, 96, OPAQUE_TEXT);
            Estado = intermedio_3;

               break;

        case intermedio_3:
            if (j == 0) // Solo reproducir la melod�a una vez
                 {
                     melodia();
                     j = 1; // Marcar que la melod�a ya se ha ejecutado
                 }

             if(!(P2IN&BIT5)) //PULSO EL BOTON PARA PASAR AL SIGUIENTE ESTADO
             {
                            Estado = Juego_3;
                            t = 0;
                            j = 0;
             }

            break;

        case Juego_3:
                    // PINTAMOS EL FONDO
                    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
                    Graphics_clearDisplay(&g_sContext);

                    // VARIABLES PARA EL RECT�NGULO (JUGADOR)
                    static int rect_x = 50;    // Posici�n inicial del rect�ngulo en X
                    int rect_width = 25;       // Ancho del rect�ngulo
                    int rect_height = 10;      // Alto del rect�ngulo
                    int rect_y = 118;          // Posici�n Y fija para estar en la parte inferior

                    // VARIABLES PARA LAS MANZANAS
                    static int manzana_x[5] = {20, 60, 40, 100, 80}; // Tres posiciones de X predefinidas
                    static int manzana_y[5] = {0, 0, 0, 0, 0};     // Las tres manzanas comienzan en Y=0
                    static int manzana_activa = 0;           // Controla qu� manzana est� activa
                    static int tiempo_generacion = 0;        // Control de tiempo para nuevas manzanas
                    static int colisiones = 0;               // Contador de colisiones (y aciertos)

                    int i; // Variable de control para bucles

                    // MOVER EL RECT�NGULO CON EL JOYSTICK
                    ejex = lee_ch(0);
                    if (ejex < 200 && rect_x > 0) {
                        rect_x -= 5; // Mover a la izquierda
                    } else if (ejex > 800 && rect_x + rect_width < 127) {
                        rect_x += 5; // Mover a la derecha
                    }

                    // DIBUJAR EL RECT�NGULO
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                    Graphics_Rectangle rect = {rect_x, rect_y, rect_x + rect_width, rect_y + rect_height};
                    Graphics_fillRectangle(&g_sContext, &rect);

                    // GENERAR NUEVA MANZANA EN SECUENCIA
                    tiempo_generacion++;
                    if (manzana_activa < 5 && tiempo_generacion >= 10) { // Medio segundo por manzana
                        manzana_y[manzana_activa] = 30; // Las manzanas empiezan a caer desde Y=30 (debajo de la l�nea)
                        manzana_activa++;
                        tiempo_generacion = 0; // Reinicia el contador de tiempo
                    } else if (manzana_activa == 5 && tiempo_generacion >= 30) { // Espera A QUE TODAS HAYAN SALIDO PARA GENERAR OTRAS 5
                        manzana_activa = 0; // Reinicia el ciclo de manzanas
                        tiempo_generacion = 0; // Reinicia el contador
                    }

                    // DIBUJAR Y MOVER LAS MANZANAS ACTIVAS HACIA ABAJO
                    for (i = 0; i < manzana_activa; i++) {
                        if (manzana_y[i] <= 127) { // Si la manzana est� dentro de la pantalla
                            Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_RED);
                            // Cambiar tama�o de la manzana a 8x8
                            Graphics_Rectangle manzana = {manzana_x[i], manzana_y[i], manzana_x[i] + 8, manzana_y[i] + 8};
                            Graphics_fillRectangle(&g_sContext, &manzana);

                            // Mover la manzana hacia abajo
                            manzana_y[i] += vel_caida_manzana; // Velocidad de ca�da (ajustable)

                            // DETECTAR COLISI�N CON EL RECT�NGULO DEL JUGADOR
                            if (manzana_y[i] + 8 >= rect_y && manzana_x[i] + 8 >= rect_x && manzana_x[i] <= rect_x + rect_width) {
                                manzana_y[i] = 128; // "Elimina" la manzana al salir de pantalla
                                colisiones++; // Incrementa el contador de colisiones (y aciertos)
                                if (j == 0) // Solo reproducir la melod�a una vez
                                     {
                                         melodia_bien();
                                         j = 1; // Marcar que la melod�a ya se ha ejecutado
                                     }
                                j=0;
                            }
                        }
                    }

                    // Mostrar el contador de aciertos en la pantalla (arriba a la izquierda)
                    sprintf(aciertos_text, "Aciertos: %d", colisiones);
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                    Graphics_drawString(&g_sContext, aciertos_text, -1, 5, 5, OPAQUE_TEXT);

                    // DIBUJAR UNA L�NEA HORIZONTAL BLANCA
                    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                    Graphics_drawLine(&g_sContext, 0, 30, 127, 30); // L�nea en Y=30

                    // VERIFICAR SI EL JUGADOR HA GANADO
                    if (colisiones >= 5)
                    {
                        // MOSTRAR PANTALLA DE VICTORIA
                        Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
                        Graphics_clearDisplay(&g_sContext);
                        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                        Graphics_drawString(&g_sContext, "ENHORABUENA", 14, 20, 20, OPAQUE_TEXT);
                        Graphics_drawString(&g_sContext, "HAS GANADO :)", 20, 10, 40, OPAQUE_TEXT);
                        //pulsa_boton();
                        Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
                           Graphics_drawString(&g_sContext,"Pulsa boton para", 16, 12, 80, OPAQUE_TEXT);
                           Graphics_drawString(&g_sContext,"seguir jugando", 14, 14, 96, OPAQUE_TEXT);


                        __delay_cycles(2000000);
                        Estado = intermedio_4;
                        break;
                    }


                    __delay_cycles(50000);
                        break;
        case intermedio_4:

            if(!(P2IN&BIT5)) //PULSO EL BOTON PARA PASAR AL SIGUIENTE ESTADO
            {
                           Estado = Nivel_1;
                           t = 0;
                           j = 0;
                           punt = 0;
                           cont_kills = 0;
                           colisiones = 0;

                           cuadros_movimiento = cuadros_movimiento + 5;
                           tiempo_flecha = tiempo_flecha-25;
                           vel_caida_manzana = vel_caida_manzana+5;
            }

            break;

        default: Estado = espera;
        case espera:
            break;
        }
}
}



//INTERRUPCION_TIMER
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMER1_300ms(void)
{
    LPM0_EXIT; //Despierta al micro al final del tiempo
    t++;
    moment++;

    if(moment>=1000)
        moment = 0;
}

#pragma vector=ADC10_VECTOR
__interrupt void ConvertidorAD(void)
{
    LPM0_EXIT; //Despierta al micro al final de la conversi�n
}
