# Notes of Peace

## Slide 1
- Hva heter jeg?
- Hva skal skje
- Navn på oppgaven

## Slide 2
- Før vi går videre:
- Falske GPS-signaler kalles spoofede signaler
- Jeg har laget et system for å gjøre GPS-styrte klokker motstandsdyktige mot falske GPS-signaler.
- GPS-mottakere er dessverre svært naive og enkle å lure
- Systemet er en prototype ment for å teste deteksjonsteknikker.
- Aldri gjort før. Lavthengende frukt.

## Slide 3
- Hva har GPS med timing å gjøre? Er ikke GPS kun ment for finne frem med bilen når en er på ferie?
- Figuren viser trilaterasjon. Får å finne vår posisjon er vi avhengige av 4 satellitter, X,Y,Z og TID. 
- Trilaterasjon = bruker mål av avstand. Triangulering bruker mål av vinkler.
- Når posisjonen er på plass, så er klokka i GPS-mottakeren vår synkronisert med atomklokkene på GPS-satellittene.
- Det gir den ganske dårlige krystalloscillatorene en langtidsstabilitet en egentlig bare kan drømme om. 
- Så greit. Vi har klokke med god langtidstabilitet. So what? Kortidstabiliteten er fremdeles begredelig!
- Vi kan bruke GPS-mottakeren til å korrigere klokker med god kortidstabilitet slik at de får bra langtidstabilitet.

## Slide 4
- Vi har noen eksempler her
- Telekommunikasjon. Her er det viktig at nettet er synkront for stabil kommunikasjon.
- Kraftproduksjon. I kraftnettet er det viktig at kraft produserer med lik fase. Alternative kan være strømbrudd. GPS gir en felles tidskilde og mulighet for synkronisering.
- Aksjehandel. Feil tidstempel gir ugyldig handel. Gjelder også handel på internett.
- Dette er bare noen eksempler.

## Slide 5 
- Så vi lever i et samfunn hvor GPS er kritisk. Men er det sikkert?	
- En GPS-mottaker er avhengig av å ha en antenne med fri sikt. Det gjør den vanskelig å sikre. Den bør også ha god "gps-geometri", som betyr at det ikke er noen god ide å hjemme den bort mellom noen høye vegger.
- Kodestrukturen i signalene fra satellitene er kjent. 
- Mottakerne er naive. En GPS-mottaker har INGEN problemer med å være i Oslo 12. Desember 2016 for så et minutt senere våre på Hawaii i midten av Juni året før.
- Hvordan narrer vi en GPS-mottaker? En måte er Jamming. Spill av et signal på samme bølgelengde.
- Spoofing som tidligere nevnt. Kan gjøres på flere måter. En kan konstruere et falskt signal som GPS-mottakeren låser seg på. 
- En kan også spille av et signal en har tatt opp. Dette gir riktig posisjon men feil tid.

## Slide 6
- I media er GPS spoofing ofte nevnt i kontekst med droner.
- Droner som bruker GPS for navigasjon kan bli spoofet til å lande på feil sted.
- Det er ikke like tabloidvennlig, men en klokke korrigert av en spoofet GPS-mottaker er også kapret!

## Slide 7
- GPS-timing kan vurderes som en ukryptert og fysisk usikret port inn i industrielle kontroll systemer.

## Slide 8
- The Apex Predator, en kan kjøpe billige GPS spoofere på ebay eller kan bruke en slik.
- Civil GPS Spoofer ble laget av et team fra University of Texas at Austin i 2012.
- Den er implementert i software definert radio og kan lage 14 falske satelittsignaler.
- Det spennende er at den bruker et autentisk signal som kilde. Når en GPS-mottakere er låst på signalet, kan signalet manipuleres i sanntid. 
- Poenget er at den skal være så skånsom og forsiktig som mulig. 
- Brukes for å manipulere tid og da spesielt i kontekst med PMUer.
- Dette er trusselen vi hadde i tankene da vi planla våres system og mottiltak.

## Slide 9
- Er alt håp ute? Bare å gi opp? Nei.
- Enkle måter å detektere og mulige mottiltak

## Slide 10 -> Du bør ha brukt 10-11 minutter nå!
- Spoofer med signalkilde og en transmitter

## Slide 11
- Spooferen skrus på
- Mottakerne vil løse samme posisjon som signalkilden.
- Vanskelig å gjennomføre spoofing av flere antenner uten at de forandrer posisjon.

## Slide 12
- Når jeg prater om en god klokke, så tenker jeg på en klokke som trenger få korreksjoner sjeldent.
- SA.45s er en Chip scale atomic clock fra Symmetricom og er et eksempel på en slik klokke. Den er også svært rimelig (5000 nok)
- Har også en intern frekvensteller og styringsalgoritme.

## Slide 13
- Dette er en ganske typisk oppbygning for atomklokker.
- Hvordan atomklokker fungerer er utenfor skopet av denne oppgaven, men grunnen til at figuren vises, er at klokka har et løkkefilter implementert på en mikrokontroller. Denne mikrokontrolleren kan en kommunisere med over RS-232 og informasjon kan hentes ut. Denne informasjonen er viktig da den gir oss viktige parametre for klokka.

## Slide 14
- Bare les krava.

## Slide 15
- Skrivefeil! Skal stå 10^-12
- Designet av Harald Hauglin, implementert av meg i C.
- Bruker data fra klokka for å bygge seg opp. Vi har satt 2 dager som tid som trengs for å gjøre den moden.
- Forklar grafen. Bakgrunnen har vi frekvensstyring som klokka har lagt på. Den svarte tynne er den midlede. Stiplede er predikert verdi fra modellen.
- Viser også hvordan klokka oppfører seg, at den ikke trenger store korreksjonene.

## Slide 16
- Filtre er algoritmer som enten bruker GPS data (NMEA) eller telemetri fra klokka til avdekke abnormale tilstander.
- Sted og hastighet og fasehopp filtrene bruker pre-konfigurerte grenseverdier
- Disse grenseverdiene ble regnet ved at vi logget data fra GPS mottakerne og klokka for å finne ut hva vi kunne regne som en normal. 
- De forskjellige filtrene er IKKE VEKTET på noen måte. 
- Optimale grenseverdier utenfor skopet av oppgaven.   
- Frekvenskorreksjonsfilteret bruker klokkemodellen som referanse. 

## Slide 17
- Siden administrering over nettverk, logging og utbyggbarhet var blant kravene, var det naturlig å også få GPS-mottakerne over på et nettverk.
- Enkeltbrettsdatamaskiner svært rimelig og en enkel måte å få en GPS-mottaker med nettverkskort
- Jeg har ikke gått på Westerdals, så jeg er komfortabel med å kalle dette en Sensor.

## Slide 18
- Klientprogramvare på Sensor.
- Skrevet i C99
- Dataene hentet ut fra GPS-mottakeren er i NMEA format

## Slide 19

## Slide 20
- Overordnet ide med roller

## Slide 21 - Slide 25
- Ta deg tid til å beskrive figurene

## Slide 26
- Plassert på taket på JV
- Ca. 30 meter fra hverandre

## Slide 27
- Vi har ikke en GPS spoofer
- Manipulerer tidsløsningen ved å flytte på mottakerne som SKAL stå på samme sted.

## Slide 28
- Baktpotet-o-rama

## Slide 29
- Forklar aksene!
- Den tykke blå er data fra CSAC
- Oransje i bakgrunnen er fra en CNT-91 frekvensteller
- Loggene fra GPS, Serveren og klokka korrelerer svært bra.

## Slide 30
- Poenget er å teste Klokkemodellen og styring

## Slide 32
- Tydelig faseavvik













