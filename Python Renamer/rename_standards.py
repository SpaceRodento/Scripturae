#!/usr/bin/env python3
"""
Skripti SFS-standardien uudelleennimeämiseen PDF-tiedostojen sisällön perusteella.
Etsii standardinumeroita PDF:ien ensimmäisiltä sivuilta ja nimeää tiedostot sen mukaan.
"""

import os
import re
import sys
from pathlib import Path

try:
    import PyPDF2
except ImportError:
    print("VIRHE: PyPDF2-kirjasto puuttuu!")
    print("Asenna se komennolla: pip install PyPDF2")
    sys.exit(1)


def extract_standard_number(pdf_path, max_pages=3):
    """
    Etsii standardinumeron PDF-tiedostosta.
    
    Args:
        pdf_path: Polku PDF-tiedostoon
        max_pages: Montako ensimmäistä sivua tutkitaan (oletus: 3)
    
    Returns:
        Standardinumero tai None jos ei löydy
    """
    try:
        with open(pdf_path, 'rb') as file:
            reader = PyPDF2.PdfReader(file)
            
            # Tutki ensimmäiset sivut
            pages_to_check = min(max_pages, len(reader.pages))
            
            for page_num in range(pages_to_check):
                try:
                    text = reader.pages[page_num].extract_text()
                    
                    # Etsi erilaisia standardiformaatteja
                    patterns = [
                        r'SFS[- ]?EN[- ]?ISO[- ]?\d+(?:[- ]\d+)*',  # SFS-EN ISO 12100 tai SFS EN ISO 12100-1
                        r'SFS[- ]?EN[- ]?\d+(?:[- ]\d+)*',           # SFS-EN 61131 tai SFS EN 61131-3
                        r'SFS[- ]?ISO[- ]?\d+(?:[- ]\d+)*',          # SFS-ISO 2768-1
                        r'SFS[- ]?\d+',                               # SFS 2045
                    ]
                    
                    for pattern in patterns:
                        matches = re.findall(pattern, text, re.IGNORECASE)
                        if matches:
                            # Ota ensimmäinen löytynyt
                            standard = matches[0]
                            # Normalisoi väliviivat
                            standard = re.sub(r'\s+', '-', standard)
                            standard = standard.upper()
                            return standard
                            
                except Exception as e:
                    print(f"Varoitus: Sivun {page_num + 1} lukeminen epäonnistui: {e}")
                    continue
            
            return None
            
    except Exception as e:
        print(f"Virhe tiedoston {pdf_path} käsittelyssä: {e}")
        return None


def rename_pdfs(directory, dry_run=True):
    """
    Nimeää kansion PDF-tiedostot uudelleen standardinumeroiden mukaan.
    
    Args:
        directory: Kansio jossa PDF-tiedostot sijaitsevat
        dry_run: Jos True, vain näyttää mitä tehtäisiin (oletus: True)
    """
    directory = Path(directory)
    
    if not directory.exists():
        print(f"VIRHE: Kansiota {directory} ei löydy!")
        return
    
    # Etsi kaikki PDF-tiedostot
    pdf_files = list(directory.glob("*.pdf"))
    
    if not pdf_files:
        print(f"Kansiosta {directory} ei löytynyt PDF-tiedostoja.")
        return
    
    print(f"Löydettiin {len(pdf_files)} PDF-tiedostoa.\n")
    
    renamed_count = 0
    failed_count = 0
    
    for pdf_file in pdf_files:
        print(f"Käsitellään: {pdf_file.name}")
        
        # Etsi standardinumero
        standard_number = extract_standard_number(pdf_file)
        
        if standard_number:
            # Luo uusi tiedostonimi
            new_name = f"{standard_number}.pdf"
            new_path = pdf_file.parent / new_name
            
            # Tarkista ettei uusi nimi ole jo käytössä
            if new_path.exists() and new_path != pdf_file:
                print(f"  ⚠️  Tiedosto {new_name} on jo olemassa, ohitetaan.")
                failed_count += 1
            else:
                if dry_run:
                    print(f"  ✓  Nimettäisiin uudelleen: {new_name}")
                else:
                    try:
                        pdf_file.rename(new_path)
                        print(f"  ✓  Nimetty uudelleen: {new_name}")
                        renamed_count += 1
                    except Exception as e:
                        print(f"  ✗  Uudelleennimeäminen epäonnistui: {e}")
                        failed_count += 1
        else:
            print(f"  ✗  Standardinumeroa ei löytynyt")
            failed_count += 1
        
        print()
    
    # Yhteenveto
    print("=" * 60)
    if dry_run:
        print("TESTIAJO - Tiedostoja ei nimetty uudelleen!")
        print(f"Nimettäisiin uudelleen: {renamed_count}")
    else:
        print(f"Onnistuneesti nimetty uudelleen: {renamed_count}")
    print(f"Epäonnistuneet/ohitetut: {failed_count}")
    print("=" * 60)


def main():
    """Pääohjelma"""
    print("=" * 60)
    print("SFS-standardien uudelleennimeämistyökalu")
    print("=" * 60)
    print()
    
    # Kysy kansio käyttäjältä
    if len(sys.argv) > 1:
        directory = sys.argv[1]
    else:
        directory = input("Anna kansion polku (tyhjä = nykyinen kansio): ").strip()
        if not directory:
            directory = "."
    
    directory = Path(directory).resolve()
    print(f"\nKäytetään kansiota: {directory}\n")
    
    # Testiajo ensin
    print("Suoritetaan ensin TESTIAJO (tiedostoja ei muuteta)...\n")
    rename_pdfs(directory, dry_run=True)
    
    # Kysy vahvistus
    print("\nHaluatko suorittaa uudelleennimeämisen oikeasti?")
    response = input("Kirjoita 'KYLLÄ' vahvistaaksesi: ").strip().upper()
    
    if response == "KYLLÄ":
        print("\nSuoritetaan uudelleennimeäminen...\n")
        rename_pdfs(directory, dry_run=False)
    else:
        print("\nPeruttu. Tiedostoja ei muutettu.")


if __name__ == "__main__":
    main()
