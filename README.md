# YAEOS
Yet Another Educational Operating System
Fase 2 del progetto YaeOS.
Anno accademico 2017/2018.
## Istruzioni di compilazione

#### Fase A
Per testare il progetto e' necessario avere installato uARM (https://github.com/mellotanica/uARM).


#### Fase B 
  - Scaricare il progetto da GitHub (https://github.com/johnMinelli/yaeOS).
  - Scompattare l'archivio.
 
#### Fase C
  - Spostarsi all'interno della cartella master (il nominativo default sarà "yaeOS-master").
  
        $ cd yaeOS-master
  
  - Eseguire il comando 'make'.
  
        $ make    
  
  - Verra' creato un file denominato "test". 
  - I file *.o verranno automaticamente rimossi.

### Fase D
  - Dopo aver correttamento compilato il file 'test' per poterlo provare eseguire uarm

        $ uarm

  - In "Settings"(Machine Configuration) > "General" > "Core file" : inserire la path del file 'test'
  - E' ora possibile accendere la macchina e osservarne l'esecuzione

### Link utili

Progetto uARM:

* [uARM Code](https://github.com/mellotanica/uARM) - Mellotanica's GitHub project

Specifiche consegna fase 2 e file utili:

* [Fase 1](http://www.cs.unibo.it/~renzo/so/yaeos/phase2/) - Renzo Davoli

Specifiche macchina emulata uARM:

* [uARM Doc](http://amslaurea.unibo.it/11866/) - tesi Melletti Marco

License
----
MIT
