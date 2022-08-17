/*El pig latín es una jerga en inglés que altera el orden de una palabra y le añade un
sufijo.
Reglas:
1. Si la palabra comienza con una consonante, se muevan todas las
consonantes antes de la primera vocal al final y se agrega la sílaba “ay”
2. Si la palabra comienza con una vocal, no se realiza ningún movimiento,
únicamente se agrega la silaba “ay”
Se solicita que genere una solución en consola con el lenguaje de programación
Java que transforme una oración (sin mayúsculas y sin contracciones) en inglés a
pig latín.
Al ejecutar el programa, en consola, se deberá pedir al usuario una oración en
inglés. Luego de ingresar la oración el programa deberá de imprimir en consola la
oración transformada en pig latín.
*///


import java.util.Scanner;
public class App {

    static Scanner sc = new Scanner(System.in);
    public static void main(String[] args) throws Exception {


        System.out.println("Ingrese una oracion en ingles sin contracciones: ");
        String cadena = sc.nextLine();
        String letras [] = cadena.toLowerCase().trim().split(" ");

        for(int i = 0; i<cadena.length(); i++){
            int id = metodoresolvedor(letras[i]);

            System.out.print(letras[i].substring(id)+ letras[i].substring(0, id)+"ay ");
        }
        sc.close();

    }

    public static int metodoresolvedor(String regresar){
        for(int i=0; i<regresar.length(); i++){
            if(regresar.charAt(i)=='a'||
            regresar.charAt(i)=='e'||
            regresar.charAt(i)=='i'||
            regresar.charAt(i)=='o'||
            regresar.charAt(i)=='u'){
                return i;
            }
        }


        return 0;
    }
}

