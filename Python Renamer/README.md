# SFS-standardien uudelleennimeämistyökalu

## Asennus

1. Varmista että Python 3 on asennettuna
2. Asenna PyPDF2-kirjasto:

```bash
pip install PyPDF2
```

## Käyttö

### Peruskäyttö

Suorita skripti kansiossa jossa PDF-tiedostot sijaitsevat:

```bash
python rename_standards.py
```

Tai anna kansion polku parametrina:

```bash
python rename_standards.py /polku/kansioon/jossa/pdfit
```

### Mitä skripti tekee?

1. Etsii kaikki PDF-tiedostot määritetystä kansiosta
2. Lukee jokaisen PDF:n ensimmäiset 3 sivua
3. Etsii standardinumeroita muodoissa:
   - SFS-EN ISO 12100
   - SFS-EN 61131-3
   - SFS-ISO 2768-1
   - SFS 2045
4. Nimeää tiedostot uudelleen löydetyn standardinumeron mukaan

### Turvallisuus

- Skripti suorittaa AINA ensin testiajan joka näyttää mitä tehtäisiin
- Tiedostoja ei muuteta ennen kuin vahvistat toiminnon kirjoittamalla "KYLLÄ"
- Jos uusi tiedostonimi on jo käytössä, alkuperäistä ei ylikirjoiteta

## Esimerkkejä

### Ennen:
```
12345678.pdf
87654321.pdf
random_name.pdf
```

### Jälkeen:
```
SFS-EN-ISO-12100.pdf
SFS-EN-61131-3.pdf
SFS-2045.pdf
```

## Vinkkejä

- Pidä varmuuskopio tiedostoista ennen uudelleennimeämistä
- Jos standardinumeroa ei löydy, tiedostoa ei nimetä uudelleen
- Voit järjestää tiedostot alikansioihin aihealueittain ennen uudelleennimeämistä

## Vianmääritys

**"PyPDF2-kirjasto puuttuu"**
→ Asenna komennolla: `pip install PyPDF2`

**"Standardinumeroa ei löytynyt"**
→ PDF saattaa olla skannaus tai teksti ei ole luettavassa muodossa
→ Nimeä nämä tiedostot käsin

**"Tiedosto on jo olemassa"**
→ Samalla numerolla on jo tiedosto, tarkista duplikaatit manuaalisesti
