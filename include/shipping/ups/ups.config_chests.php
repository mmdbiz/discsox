;<?php if(!defined('INIT.CART')){die( "<html><head><title>403 Forbidden</title></head><body><h2>403 Forbidden</h2><p>You don't have permission to access this resource.<p></body></html>" );}?>
	; do not remove line above. It's used so no one can read this file from the web.
    ;
    ; UPS (XML) configuration File:
    ;
    ; NOTE: This configuration file is used for both the standard and xml versions of the UPS plug-ins
    ;
[settings]
    ;
    ; Access info:
    ;
    ; XML Access Key (XML Version Only)
    ;
AccessLicenseNumber=EC150F5CCBC94EB0
    ;
    ; UPS Username  (XML Version Only)
    ;
UserId=mmdesign
    ;
    ; UPS Developer Key (XML Version Only)
    ;
Password=5B571BA055F79508
    ;
    ; Origin Postal Code:
    ; (Leave blank if country is not US)
    ;
originPostalCode=19804
    ;
    ; Country Info (International Requests)
    ; (These MUST be filled in if not US)
    ;
originCountryCode=US
    ;
    ; Ship From City
    ;
originCity=Wilmington
    ;
    ; UPS API url:
    ;
RateURL=https://www.ups.com/ups.app/xml/Rate
    ;
    ; UPS Rate chart values:<br><br>
    ;
    ; 01 = Daily Pickup<br>
    ; 03 = Customer Counter<br>
    ; 06 = One Time Pickup<br>
    ; 07 = On Call Air<br>
    ; 19 = Letter Center<br>
    ; 20 = Air Service Center<br><br>
    ;
    ; Enter the number ONLY from above:<br>
    ;
PickupType=01
    ;
    ; Type of packaging:<br><br>
    ;
    ; 00,Unknown<br>
    ; 01,UPS letter<br>
    ; 02,Package<br>
    ; 03,UPS Tube<br>
    ; 04,UPS Pak<br>
    ; 21,UPS Express Box<br>
    ; 24,UPS 25KG Box<br><br>
    ;
    ; <b>NOTE:</b> MUST include both code and description ( 02,Package )<br>
    ;
PackagingType=02,Package
    ;
    ; default_package_size:
    ;
default_package_length=20
default_package_width=10
default_package_height=6
    ;
    ; Services<br><br>
    ;
    ; 01 = Next Day Air<br>
    ; 02 = 2nd Day Air<br>
    ; 03 = Ground<br>
    ; 07 = Worldwide Express<br>
    ; 08 = Worldwide Expedited<br>
    ; 11 = Standard<br>
    ; 12 = 3-Day Select<br>
    ; 13 = Next Day Air Saver<br>
    ; 14 = Next Day Air Early AM<br>
    ; 54 = Worldwide Express Plus<br>
    ; 59 = 2nd Day Air AM<br>
    ; 65 = Express Saver<br><br>
    ;
    ; <b>NOTE:</b> Use 11 for rate shopping (show all rates and services)<br><br>
    ;
    ; Enter the number ONLY from above:<br>
    ;
ServiceCode=11
    ;
    ; Rate Request Options:
    ;    rate = one single rate
    ;    shop = all available rates
    ;
RequestOption=shop
    ;
    ; Handling Charge to be added to results from UPS
    ;
handling_charge=0.00
    ;


[services]
    ;
    ; NOTE: comment out the ones you do not want displayed using a semi-colon
    ;
3=UPS Ground
;2=UPS 2nd Day Air
;1=UPS Next Day Air
;7=UPS Worldwide Express
;8=UPS Worldwide Expedited
;11=UPS Standard
;12=UPS 3 Day Select
;13=UPS Next Day Air Saver
;14=UPS Next Day Air Early A.M.
;54=UPS Worldwide Express Plus
;59=UPS 2nd Day Air A.M.
;65=UPS Express Saver













