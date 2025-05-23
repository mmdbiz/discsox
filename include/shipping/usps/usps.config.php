;<?php if(!defined('INIT.CART')){die( "<html><head><title>403 Forbidden</title></head><body><h2>403 Forbidden</h2><p>You don't have permission to access this resource.<p></body></html>" );}?>
	; do not remove line above. It's used so no one can read this file from the web.
    ;
    ; USPS (XML) configuration File:
    ;
[settings]
    ;
    ; User account information
    ;
usps_user_id=739MMDES6091
    ;
usps_password=733VZ53CE514
    ;
    ; Default ship From zip code
    ;
usps_origin_postal_code=94579
    ;
    ; Default Package Container Type: (For someone using their own package, use NONE)
    ; marcello this has changed with RateV4/2 use RECTANGULAR
    ;
default_usps_container=RECTANGULAR
    ;
    ; Default Package Size: REGULAR, LARGE or OVERSIZE
    ;
default_package_size=REGULAR
    ;
    ; Machinable: TRUE or FALSE
    ;
default_machinable="True"
    ;
    ; A handling charge to add to the results
    ;
handling_charge=0.00
    ;
    ; Selected Services to display: (This is a simple, comma delimited list):
    ; Priority,Express,Parcel,First Class
    ;
usps_services=Priority,Express,Parcel
;usps_services=Express
    ;
    ; The default service that is selected when the
    ; result page is displayed for domestic shipments.
    ;
default_usps_service=Express
    ;
    ; The default service that is select when the
    ; result page is displayed for International Shipments.
    ;
default_international_usps_service=Express
    ;
    ; Should we add insurance to the rates?
    ;
add_insurance=false
	;	
[free shipping]
    ;
    ; Free shipping allows you to offer free shipping for orders
    ; over a specific amount.
    ;
offer_free_shipping=false
    ;
    ; States to exclude from free shipping
    ;
exclude_states=AK,HI
    ;
    ; The required minimum subtotal to get free shipping
    ;
free_shipping_subtotal=50.00
	;
free_shipping_service=Priority Mail Express 2-Day
	;
	; The text displayed to the user
    ;
free_shipping_text=Free Priority Mail

[services to display]
	;
	; (set the ones you want displayed to: true)
	;
	; Domestic services:
	; 
First-Class Mail=false
Express Mail PO to PO=false
Express Mail=true
Priority Mail Express=true
Priority Mail Express 1-Day=true
Priority Mail Express 2-Day=true
Express Mail Flat-Rate Envelope=false
Priority Mail=true
Priority Mail 1-Day=true
Priority Mail 2-Day=true
Priority Mail Express 1-Day Flat Rate Envelope=false
Priority Mail 1-Day Flat Rate Envelope =false
Priority Mail 2-Day Flat Rate Envelope =false
Priority Mail 3-Day Flat Rate Envelope =false
Priority Mail Padded Flat Rate Envelope =true
Priority Mail 1-Day Padded Flat Rate Envelope =true
Priority Mail 2-Day Padded Flat Rate Envelope =true
Priority Mail 3-Day Padded Flat Rate Envelope =true
Priority Mail Flat Rate Box=false
USPS Retail Ground=true
Media Mail Parcel=false
Library Mail Parcel=false
	;
	; International services:
	;
First-Class Mail International Parcel**=false
	;
Priority Mail International=true
Priority Mail International Flat Rate Box=false
Priority Mail Express International=true
	;
Express Mail International=true
Express Mail International (EMS) Flat Rate Envelope=true
	;
Global Express Guaranteed (GXG)=false
Global Express Guaranteed (GXG) Non-Document Rectangular=false
Global Express Guaranteed (GXG) Non-Document Non-Rectangular=false
	;

[insurance rates]
    ;
    ; for subtotals over $600 the insurance fee will be:
    ; $7.20 + $1.00 for every $100 or fraction of $100 above $600.
    ; counter=low range,high range,insurance fee
    ;
01=0.01,50,1.30
02=50.01,100,2.20
03=100.01,200,3.20
04=200.01,300,4.20
05=300.01,400,5.20
06=400.01,500,6.20
07=500.01,600,7.20
08=600.01,5000,7.20
    ;
[country order package weights]
	;
Australia=43
Austria=66
Belgium=70
Brazil=65
Canada=65
Colombia=44
China=70
Czech Republic=65
United Kingdom (Great Britain)=65
Finland=65
France=65
Germany=65
Greenland=65
Holland (Netherlands)=44
Hong Kong=45
Hungary=65
Iceland=65
Ireland=65
Japan=65
Mexico=44
Netherlands=44
Norway=65
Russia=44
Scotland=65
South Africa=65
South Korea=44
Sweden=65
Switzerland=65
Turkey=66
Vanuatu=44
	;

[country codes]
    ;
    ; USPS International country codes/names
	; You can check the names against https://postcalc.usps.com 
	; USPS keeps changing names!!! Leave abbreviations alone
    ;
AL=Albania
DZ=Algeria
AS=American Samoa
AD=Andorra
AI=Anguilla
AG=Antigua and Barbuda
AR=Argentina
AW=Aruba
AU=Australia
AT=Austria
AP=Azores
AP=Azores (Portugal)
BS=Bahamas
BH=Bahrain
BD=Bangladesh
BB=Barbados
BY=Belarus
BE=Belgium
BZ=Belize
BJ=Benin
BM=Bermuda
BO=Bolivia
BL=Bonaire (Netherlands Antilles)
BW=Botswana
BR=Brazil
VG=British Virgin Islands
BN=Brunei Darussalam
BG=Bulgaria
BF=Burkina Faso
BI=Burundi
KH=Cambodia
CM=Cameroon
CA=Canada
CE=Canary Islands (Spain)
KY=Cayman Islands
CF=Central African Republic
TD=Chad
NN=Channel Islands (Jersey, Guernsey, Alderney and Sark) (Great Britain)
CL=Chile
CO=Colombia
CG=Congo (Brazzaville),Republic of the
CG=Congo, Democratic Republic of the
CK=Cook Islands (New Zealand)
CR=Costa Rica
HR=Croatia
CB=Curacao (Netherlands Antilles)
CY=Cyprus
CZ=Czech Republic
DK=Denmark
DJ=Djibouti
DM=Dominica
DM=Dominican Republic
EC=Ecuador
EG=Egypt
SV=El Salvador
EN=England (United Kingdom of Great Britain and Northern Ireland)
ER=Eritrea
EE=Estonia
ET=Ethiopia
FM=Federated States of Micronesia
FJ=Fiji
FI=Finland
FR=France
GF=French Guiana
PF=French Polynesia
GA=Gabon
GM=Gambia
GE=Georgia, Republic of
DE=Germany
GH=Ghana
GI=Gibraltar
GR=Greece
GL=Greenland
GD=Grenada
GP=Guadeloupe
GU=Guam
GT=Guatemala
GN=Guinea
GN=Guinea-Bissau
GY=Guyana
HT=Haiti
HO=Holland (Netherlands)
HN=Honduras
HK=Hong Kong
HU=Hungary
IS=Iceland
IN=India
ID=Indonesia
IE=Ireland
IL=Israel
IT=Italy
JM=Jamaica
JP=Japan
JO=Jordan
KZ=Kazakhstan
KE=Kenya
KI=Kiribati
KO=Kosrae, Micronesia
KW=Kuwait
KG=Kyrgyzstan
LA=Laos
LV=Latvia
LB=Lebanon
LS=Lesotho
LR=Liberia
LI=Liechtenstein
LT=Lithuania
LU=Luxembourg
MO=Macau (Macao)
MK=Macedonia, Republic of
MG=Madagascar
ME=Madeira Islands (Portugal)
MW=Malawi
MY=Malaysia
MV=Maldives
ML=Mali
MT=Malta
MH=Marshall Islands
MQ=Martinique
MR=Mauritania
MU=Mauritius
MX=Mexico
MD=Moldova
MC=Monaco (France)
MS=Montserrat
MA=Morocco
MZ=Mozambique
MM=Myanmar (Burma)
NA=Namibia
NP=Nepal
NL=Netherlands
CW=Netherlands Antilles (Curacao)
SX=Netherlands Antilles (Sint Maarten)
NC=New Caledonia
NZ=New Zealand
NI=Nicaragua
NE=Niger
NE=Nigeria
NF=Norfolk Island (Australia)
NB=Northern Ireland (United Kingdom of Great Britain and Northern Ireland)
MP=Northern Mariana Islands, Commonwealth of
NO=Norway
OM=Oman
PK=Pakistan
PW=Palau
PA=Panama
PG=Papua New Guinea
PY=Paraguay
PE=Peru
PH=Philippines
PL=Poland
PT=Portugal
;PR=Puerto Rico
QA=Qatar
RE=Reunion
RO=Romania
RT=Rota, Northern Mariana Islands (US Possession)
RU=Russia
RW=Rwanda
SS=Saba (Netherlands Antilles)
SS=Sabah (Malaysia)
SP=Saipan, Northern Mariana Islands (US Possession)
SA=Saudi Arabia
SF=Scotland (United Kingdom of Great Britain and Northern Ireland)
SN=Senegal
RS=Serbia
SC=Seychelles
SL=Sierra Leone
SG=Singapore
SI=Slovenia
SB=Solomon Islands
ZA=South Africa
KR=South Korea (Korea, Republic of)
ES=Spain
LK=Sri Lanka
SW=St. Christopher and Nevis
EU=St. Eustatius (Netherlands Antilles)
LC=St. Lucia
SX=St. Maarten (Netherlands Antilles)
TB=St. Martin (Guadeloupe)
VC=St. Vincent and the Grenadines
SD=Sudan
SR=Suriname
SZ=Swaziland
SE=Sweden
CH=Switzerland
SY=Syrian Arab Republic
TA=Tahiti (French Polynesia)
TW=Taiwan
TJ=Tajikistan
TZ=Tanzania
TH=Thailand
TI=Tinian, Northern Mariana Islands(US Possession)
TG=Togo
TO=Tonga
TO=Tongareva (New Zealand)
TT=Trinidad and Tobago
TU=Truk (See Chuuk Island)
TN=Tunisia
TR=Turkey
TC=Turks and Caicos Islands
TV=Tuvalu
UG=Uganda
UA=Ukraine
AE=United Arab Emirates
;GB=United Kingdom (Great Britain)
GB=United Kingdom of Great Britain and Northern Ireland
UY=Uruguay
UZ=Uzbekistan
VU=Vanuatu
VE=Venezuela
VN=Vietnam
WK=Wake Island
WL=Wales (Great Britain and Northern Ireland)
WF=Wallis and Futuna Islands
WS=Western Samoa
YA=Yap, Micronesia
ZM=Zambia
ZW=Zimbabwe

