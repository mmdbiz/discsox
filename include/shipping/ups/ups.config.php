;<?php if(!defined('INIT.CART')){die( "<html><head><title>403 Forbidden</title></head><body><h2>403 Forbidden</h2><p>You don't have permission to access this resource.<p></body></html>" );}?>
	; do not remove line above. It's used so no one can read this file from the web.
    ;
    ; UPS OAuth 2.0 Configuration File
    ;
[oauth_settings]
    ; OAuth 2.0 Credentials from UPS Developer Portal
    client_id=C2Fo4SZM3gHd8vBDZkStD9rq0SWjygGhbEZqoAWdSvGL7dx4
    client_secret=lo4s1wY4exBkWY92vhnAK7hAbsECxhuGRZpFol3GhJ6o7bOxaGUWkKxZDeaJIkvB
    
    ; OAuth endpoints
    token_endpoint=https://onlinetools.ups.com/security/v1/oauth/token
    
    ; API endpoints
    rate_endpoint=https://onlinetools.ups.com/api/rating/v1/Shop

[settings]
    ; Origin Info
    originPostalCode=94587
    originCountryCode=US
    originCity=Union City
    
    ; Pickup Type and Packaging settings remain the same
    PickupType=01
    PackagingType=02,Package
    
    ; Package defaults
    default_package_length=20
    default_package_width=10
    default_package_height=6
    
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
2=UPS 2nd Day Air
1=UPS Next Day Air
7=UPS Worldwide Express
8=UPS Worldwide Expedited
11=UPS Standard
12=UPS 3 Day Select
13=UPS Next Day Air Saver
14=UPS Next Day Air Early A.M.
54=UPS Worldwide Express Plus
59=UPS 2nd Day Air A.M.
65=UPS Express Saver
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
free_shipping_service=UPS Ground
	;
	; The text displayed to the user
    ;
free_shipping_text=Free UPS Ground
