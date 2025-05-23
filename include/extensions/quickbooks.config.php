;<?php if(!defined('INIT.CART')){die( "<html><head><title>403 Forbidden</title></head><body><h2>403 Forbidden</h2><p>You don't have permission to access this resource.<p></body></html>" );}?>
	; do not remove line above. It's used so no one can read this file from the web.
    ;
    ; QuickBooks configuration File:
    ;
    ; Should we write the QuickBooks order files
	;
write_xml_files=true
	;    
    ; What is the name of the file to save the data to?
    ;
    ; You should use ORDERNUM in the file name. This way the
    ; QuikStore order number will be substituted into the file
    ; name when it's saved, making it unique.
    ;
xml_file_name=order.ORDERNUM.xml
	;
    ; QuickBooks Transaction type.
    ;
    ; INVOICE -or- CASH SALE (for a sales receipt)
    ;
transaction_type=CASH SALE
    ;
    ; SALES TAX:
    ;
    ; Taxable sales tax code in QuickBooks. Usually "Tax".
    ;
sales_tax_code=Tax
    ;
    ; NON taxable item
    ;
non_taxable_item=Out of State
    ;
    ; SHIPPING ITEMS:
    ;
    ; What is the name of your QuickBooks "Shipping" item.
    ;
qb_shipping_item=SnH
    ;
    ; Should we apply tax to shipping?
    ;
shipping_is_taxable=true
    ;
    ; DISCOUNT ITEMS:
    ;
    ; What is the description you want to use for a
    ; discount line item. Shows up in the invoice or
    ; sales receipt if applicable.
    ;
qb_discount_desc=Promodiscount
    ;
    ; The sales_rep entry below is used when there is no
    ; affiliate_id available. The affiliate_id will always
    ; override this setting when it's available.
    ; 5 Characters long, maximum.
    ;
sales_rep=NET
    ;
    ; The Customer Type is set to "Retail" per default
    ;
customer_type=Retail
    ;
    ; The format of the customer name used in QuickBooks:
    ; NOTE: The customer name is used as the key when the
    ; order is imported. So, it must be an exact match or
    ; a new customer will be created for the order. That's
    ; why it's important that this format match your existing
    ; customer entry format.
    ;
    ;   Last_Name,First_Name
    ;    - or -
    ;   First_Name Last_Name
    ;
customer_name_format=billaddress_lastname,billaddress_firstname
    ;
    ;
    ;Marcello's extensions
    ;The A standard message such as “Thank you for your business,
    ;” or “Please sign and return this estimate to indicate your approval.”
    ; A customer message can be included at the bottom of a form. 
    ;Note that the message has to exist in QuickBooks!!!
    ;
customer_message=Please visit https://mmdesign.com/returns.php for DiscSox Return Policy and Instructions
    ;
    ;Set the default class
    ;
default_class=DiscSox
    ;
    ;Specify the account from the account list where you want  
    ;the funds to be deposited for Credit Cards
    ;Note that the account has to exist in QuickBooks!!!
deposit_to_account_cc=Merchant
    ;
    ;Specify the account from the account list where you want  
    ;the funds to be deposited for Check Orders
    ;Note that the account has to exist in QuickBooks!!!
deposit_to_account_check=Undeposited Funds
    ;
    ;Specify the account from the account list where you want  
    ;the funds to be deposited for Cash Orders
    ;Note that the account has to exist in QuickBooks!!!
deposit_to_account_cash=Checking
    ;
    ;Specify the account from the account list where you want  
    ;the funds to be deposited for PayPal Orders
    ;Note that the account has to exist in QuickBooks!!!
deposit_to_account_pp=PayPal-DiscSox
    ;
    ;Specify the type of items you have in QuickBooks  
    ;For regular inventory items set item_type to "Inventory"
    ;For Group items set item_type to "Group"
item_type=Group
    ;specify SKUs (items) that should be ignored but not their options
ignore_skus=TP 
    ;Specify words that should be ignored in options
    ;!!!!!!!!!!!IMPORTANT!!!!!!!!!!!!! 
    ;the following words cannot be included: null, yes, no, true, false, on, off, none
    ;the following characters cannot be included: ?{}|&~![()^"
ignore_words=Palm Download Standard Mobile Edition CueCat Type Selected PS2 USB Dividers or Music Movie Labels One Free Metal Transparent Tray Styrene Divider Set Color Oil-Stain Unfinished Natural Medium Dark Black Satin Lacquer Finish Matte Nickel Weathered Antique Brass Solid Oak Handle Masonite Back Caster Base in 1 Lock 2 3 5 6 7 8 9 : J-Box Sleeves 24 25-Pack Locks Same key all drawers Different keys each drawer Shipping for SoxChest only 1000 700 600 300 CD Pro and Classic DVD DVD2 Video Data Double DataFP Sleeves Stopper Stoppers Wedges Wedge to Alabama Alaska Arizona Arkansas California Colorado Connecticut Delaware District of Columbia Florida Georgia Hawaii Idaho Illinois Indiana Iowa Kansas Kentucky Louisiana Maine Maryland Massachusetts Michigan Minnesota Mississippi Missouri Montana Nebraska Nevada New Hampshire New Jersey New Mexico New York North Carolina North Dakota Ohio Oklahoma Oregon Pennsylvania Puerto Rico Rhode Island South Carolina South Dakota Tennessee Texas Utah Vermont Virginia Washington West Virginia Wisconsin Wyoming Non US
; 
    ;Specify Items who's options should be ignored
;ignore_items=SXCH2DR SXCH_rl RWDVD_rl RWBK_rl RWCD_rl RWPP_rl MCAB2DR MCAB3DR CAD CADWRAP MCTOP
ignore_items= RWDVD_rl RWBK_rl RWCD_rl RWPP_rl MCAB3DR8W MCAB2DR MCAB2DR6 MCAB3DR MCAB4DR MCAB2DRV MCAB3DRV MCAB4DRV CAD CADWRAP MCTOP SL12 SL10 CH2DCD CH2DDVD CH3DCD CH3DDVD CH2DDVDV CH2DCDV
    ;Specify Items that should be priced in options --- All options except oak handles are priced in the meantime so don't use.
;priced_items=DVS19DIV DLT19DIV TPDP TPCD_rl TPDVD_rl 12TRCL_rl 12TRDJ_rl 12TRDVD_rl 12TRCL 12TRCDP 12TRDVD GC_Printed 12TR2 TSFDP-Front TSFDP-Back TSFDP-Short-Base TSFDP-Long-Base BLKSXCH_rl LCQ_rl SI-46-164_rl AMR-BP19202WN_rl SI-40-128_rl CHST_rl OAKB_rl CSB_rl CSBBLK_rl LOCKS_rl SnH 25PDPP 25PHDPP 25PHDP 25PHDP_s 25PGPPS3 25PGPP 25PCDPP 25PCLP 25PD2P 25PVCDP 25PD2P 25PDS 25PDSFP 25PDSD 5DIVDP 24DIVDP 5HDPBL 24DIVHDP 5DIVCP 24DIVCP 5DIVCL 24DIVCL 5DIVDVD 24DIVDVD 96MVCL_rl 96MSCL_rl T12C T12M T12MDP RBCD DCPDP DCPCP DCTDP DCTCP DCLDP DCLCP DCGDP DCGCP WDG_UNI WDG_CHST DJACP DJADP DJCP DJD2 DJDP MCCP MCDP MCECP MCEDP
    ;Specify Items that should be free in options 
free_options=P10-DP-2106O
    ;Specify Items which quantity should be used in determining quantity in options 
items_drive_option_qty=TSFDPC T12MDPC T12CC T12MC SXCH2DR SXCH_rl
