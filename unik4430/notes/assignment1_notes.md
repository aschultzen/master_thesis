# Assignment 1 notes

## Bregni 5.1
*Autonomous clocks* supply a timing signal which is generated from an oscillator internally. This oscillator is independent of external influence. A *slave clock* is a clock where the oscillator is controlled by an external reference. 

## Bregni 5.4
**NOTE: Why not atomic slave when quartz?**
Some examples of *autonomous clocks* are rubidium, caesium-beam and hydrogen-MASER oscillators. These are atomic frequency standards. The crystal quartz oscillator is also an autonomous clock, though not a atomic frequency standard. The quartz and rubidium can also be locked at a reference signal and thus work as slave clock.  

### Frequency offset
*Frequency offset is the difference between a measured frequency and an ideal frequency with zero uncertainty. This ideal frequency is called the nominal frequency.*
I believe the offset is the difference between two frequencies, for example:

	Frq A = 100 Hz
	Frq B = 120 Hz
	Offsett = B - A = 20 Hz

The offset would be static.

### Frequency drift
The stability of the frequency. How much it differs unintentionally. 

## Bregni 5.8
Time-domain analysis is efficient in providing meaningful of long-term performance. 

# Assignment 1 Answers
### Del 1
Det sier noe om nøyaktigheten over tid. GPS hadde et svært lavt stigninstall selvom signalet er stokastisk. Det er tydeligvis ustabilt men nøyaktig over lang tid. Det samme kan ikke sies om *ocxo* og *xcsac* som begge er svært stabile i forhold til GPS men som blir unøyaktige etter kort tid.

### Del 2
Det ser ut til at *xcsacen* er mest stabil, men drifter mer enn GPS som knapt drifter i forhold til *xcsac* og *ocxo*. På nært hold blir det vanskelig å se da oppløsningen for *xcsacen* er lavere grunnet færre punkter. *xcsacen* Virker å være mer stabil og nøyaktig enn *ocxoen* jevnt over. Det virker å være en klar sammenheng mellom *slope* under **Phase Difference** og *origin/drift* for **Frequency Difference**. Lavere slope gir lavere drift.

### Del 3
GPS er mest stabil. Csac er stabil lengre enn crystal. <- Prat med Harald.




