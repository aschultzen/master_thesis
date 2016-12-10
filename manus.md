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
- Det er ikke like tabloidvennlig, men en klokke korrigert av en spoofet GPS-mottaker vil kunne lures til å la seg korrigere ukorrekt.

## Slide 7
- GPS-timing kan vurderes som en ukryptert og fysisk usikret port inn i industrielle kontroll systemer.

## Slide 8






