<?php
/*
This file is run at the end of check for updates
and adds or alters tables that need updating
*/


$sessionFields = $_DB->getFieldProperties('sessions');
if(!isset($sessionFields['sort_order'])){
	$_DB->execute("ALTER TABLE `sessions` ADD `ses_cid` VARCHAR( 32 ) NULL AFTER `ses_id`");
}

// new or modified category fields		
$categoriesFields = $_DB->getFieldProperties('categories');

if(!isset($categoriesFields['sort_order'])){
	$_DB->execute("ALTER TABLE `categories` ADD `sort_order` INT( 4 ) NULL DEFAULT '0'");
}

if(isset($categoriesFields['category_description'])){
	$_DB->execute("ALTER TABLE `categories` CHANGE `category_description` `category_meta_description` TEXT NOT NULL");
	$categoriesFields['category_meta_description'] = 1;
}
if(!isset($categoriesFields['category_meta_description'])){
	$_DB->execute("ALTER TABLE `categories` ADD `category_meta_description` TEXT NOT NULL");
}
if(!isset($categoriesFields['category_meta_keywords'])){
	$_DB->execute("ALTER TABLE `categories` ADD `category_meta_keywords` TEXT NOT NULL");
}
if(!isset($categoriesFields['category_page_title'])){
	$_DB->execute("ALTER TABLE `categories` ADD `category_page_title` TEXT NOT NULL");
}
if(!isset($categoriesFields['apply_to_subcategories'])){
	$_DB->execute("ALTER TABLE `categories` ADD `apply_to_subcategories` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'false' AFTER `category_discount`");
}
if(isset($categoriesFields['category_discount'][1]) && $categoriesFields['category_discount'][1] != "varchar(255)"){
	$_DB->execute("ALTER TABLE `categories` CHANGE `category_discount` `category_discount` VARCHAR( 255 ) NOT NULL DEFAULT '0%'");
}

if(isset($categoriesFields['category_description'])){
	$_DB->execute("ALTER TABLE `categories` CHANGE `category_description` `category_meta_description` TEXT");
}
if(!isset($categoriesFields['category_meta_keywords'])){
	$_DB->execute("ALTER TABLE `categories` ADD `category_meta_keywords` TEXT NULL");
	$_DB->execute("ALTER TABLE `categories` ADD `category_page_title` TEXT NULL");
}
unset($categoriesFields);

// update lengths on link fields
$productFields = $_DB->getFieldProperties('products');

if(!isset($productFields['sort_order'])){
	$_DB->execute("ALTER TABLE `products` ADD `sort_order` INT( 4 ) NULL DEFAULT '0'");
}

if(isset($productFields['link_page'][1]) && $productFields['link_page'][1] != "varchar(255)"){
	$_DB->execute("ALTER TABLE `products` CHANGE `link_page` `link_page` VARCHAR( 255 ) NULL");
	$_DB->execute("ALTER TABLE `products` CHANGE `link_text` `link_text` VARCHAR( 255 ) NULL");
}

if(!isset($productFields['download_url'])){
	$_DB->execute("ALTER TABLE `products` ADD `download_url` VARCHAR( 255 ) NULL AFTER `download_filename`");
	$_DB->execute("ALTER TABLE `products` ADD `download_expire_days` INT( 2 ) NULL DEFAULT '0' AFTER `download_url`");
	$_DB->execute("ALTER TABLE `products` ADD `max_download_count` INT( 2 ) NULL DEFAULT '0' AFTER `download_expire_days`");
}

unset($productFields);

$downloadFields = $_DB->getFieldProperties('downloads');
if(!isset($downloadFields['url'])){
	$_DB->execute("ALTER TABLE `downloads` ADD `url` VARCHAR( 255 ) NULL AFTER `filename`");
	$_DB->execute("ALTER TABLE `downloads` ADD `expire_date` DATE NULL");
	$_DB->execute("ALTER TABLE `downloads` ADD `dl_count` INT( 10 ) NULL DEFAULT '0'");
	$_DB->execute("ALTER TABLE `downloads` ADD `max_download_count` INT( 2 ) NULL DEFAULT '0' AFTER `dl_count`");
	$_DB->execute("ALTER TABLE `downloads` CHANGE `filename` `filename` VARCHAR( 100 ) NULL");
	$_DB->execute("ALTER TABLE `downloads` ADD `sku` VARCHAR( 50 ) NOT NULL AFTER `order_number`,ADD `name` VARCHAR( 255 ) NOT NULL AFTER `sku`");
}


// update lengths on custom fields
$customFields = $_DB->getFieldProperties('product_custom');
$MetaFlds = array('detail_page_title','detail_meta_keywords','detail_meta_description');
if(isset($categoriesFields['detail_page_title'][1]) && $categoriesFields['detail_page_title'][1] != 'text'){
	foreach($MetaFlds as $j=>$fld){
		$asql = "ALTER TABLE `product_custom` CHANGE `$fld` `$fld` TEXT NULL";
		$rs = $_DB->execute($asql);
		$_DB->free_result($rs);
	}
}
else{
	// Add new meta tag fields
	foreach($MetaFlds as $j=>$fld){
		if(!isset($customFields[$fld])){
			$asql = "ALTER TABLE `product_custom` ADD `$fld` TEXT NOT NULL";
			$rs = $_DB->execute($asql);
			$_DB->free_result($rs);
		}
	}
}
unset($customFields);



// discount fields for category
$discountFlds = $_DB->getFieldProperties('discounts');
if(!isset($discountFlds['category'])){
	$_DB->execute("ALTER TABLE `discounts` ADD `category` VARCHAR( 255 ) NOT NULL AFTER `remove_after_use`");
}
if(!isset($discountFlds['catid'])){
	$_DB->execute("ALTER TABLE `discounts` ADD `catid` INT( 10 ) NULL AFTER `remove_after_use`");
}
if(!isset($discountFlds['apply_to_subcategories'])){
	$_DB->execute("ALTER TABLE `discounts` ADD `apply_to_subcategories` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'true' AFTER `category`");
}
if(isset($discountFlds['category'][2]) && strtolower($discountFlds['category'][2]) != "yes"){
	$dFieldChange  = "ALTER TABLE `discounts` CHANGE `name` `name` VARCHAR( 100 ) NULL ,
	CHANGE `coupon` `coupon` VARCHAR( 32 ) NULL ,
	CHANGE `remove_after_use` `remove_after_use` ENUM( 'true', 'false' ) NULL DEFAULT 'false',
	CHANGE `catid` `catid` INT( 10 ) NULL DEFAULT '0',
	CHANGE `category` `category` VARCHAR( 255 ) NULL ,
	CHANGE `apply_to_subcategories` `apply_to_subcategories` ENUM( 'true', 'false' ) NULL DEFAULT 'true',
	CHANGE `subtotal_ranges` `subtotal_ranges` TEXT NULL ,
	CHANGE `qty_ranges` `qty_ranges` TEXT NULL ,
	CHANGE `free_shipping` `free_shipping` ENUM( 'true', 'false' ) NULL DEFAULT 'false'";
	$_DB->execute($dFieldChange);
}
unset($discountFlds);


$_DB->execute("ALTER TABLE `help` CHANGE `section_help` `section_help` TEXT NULL");
$_DB->execute("ALTER TABLE `help` CHANGE `key_help` `key_help` TEXT NULL");

// This is an update to add the maintenance_mode variables to config
if(!isset($_CF['basics']['maintenance_mode'])){
	$_REQUEST['section'] = "basics";
	$_REQUEST['key'] = 'maintenance_mode';
	$_REQUEST['value'] = 'false';
	$_REQUEST['key_help'] = 'Allows you to turn the store off for maintenance.';
	$_Config->addEntry();
}

// This is an update to add the maintenance_mode variables to config
if(!isset($_CF['basics']['save_credit_card_info_in_database'])){
	$_REQUEST['section'] = "basics";
	$_REQUEST['key'] = 'save_credit_card_info_in_database';
	$_REQUEST['value'] = 'true';
	$_REQUEST['key_help'] = 'Allows you to turn on/off the saving of credit card information in the database.';
	$_Config->addEntry();
}

if(!isset($_CF['product_display']['sort_menu_by_fields'])){
	global $_Config;
	$_REQUEST['section'] = 'product_display';
	$_REQUEST['key'] = 'sort_menu_by_fields';
	$_REQUEST['value'] = 'sort_order';
	$_REQUEST['key_help'] = 'Allows you to define a sort by field for the menu.';
	$_Config->addEntry();
}

// add sort by fields to config
if(!isset($_CF['product_display']['sort_by_fields'])){
	global $_Config;
	$_REQUEST['section'] = 'product_display';
	$_REQUEST['key'] = 'sort_by_fields';
	$_REQUEST['value'] = 'category_name,products.name';
	$_REQUEST['key_help'] = 'Allows you to define the sort fields of the products display.';
	$_Config->addEntry();
}

// add show sub categories field to config
if(!isset($_CF['product_display']['show_subcategory_thumbnails'])){
	global $_Config;
	$_REQUEST['section'] = 'product_display';
	$_REQUEST['key'] = 'show_subcategory_thumbnails';
	$_REQUEST['value'] = 'false';
	$_REQUEST['key_help'] = 'Allows you to display a list of subcategory thumbnails when a product search returns no results and there are subcategories with products available.';
	$_Config->addEntry();
}

// add keywords field to config
if(!isset($_CF['product_display']['keyword_fields'])){
	global $_Config;
	$_REQUEST['section'] = 'product_display';
	$_REQUEST['key'] = 'keyword_fields';
	$_REQUEST['value'] = 'products.name,products.description';
	$_REQUEST['key_help'] = 'Allows you to create a list of keyword fields for a product search. Be sure to add the table name to the front of each field like products.name or product_custom.user_1';
	$_Config->addEntry();
}


// add date format and area code field to config
if(!isset($_CF['basics']['date_format'])){
	$_REQUEST['section'] = "basics";
	$_REQUEST['key'] = 'date_format';
	$_REQUEST['value'] = 'm/d/Y';
	$_REQUEST['key_help'] = 'Allows you to define the order date format.';
	$_Config->addEntry();
	$_DB->execute("UPDATE `config` SET `sequence` = 12 WHERE `section` = 'basics' AND `key` = 'maintenance_mode'");
	$_DB->execute("UPDATE `config` SET `sequence` = 10 WHERE `section` = 'basics' AND `key` = 'date_format'");
}
if(!isset($_CF['basics']['always_display_area_code'])){
	$_REQUEST['section'] = "basics";
	$_REQUEST['key'] = 'always_display_area_code';
	$_REQUEST['value'] = 'false';
	$_REQUEST['key_help'] = 'Allows you to display the telephone area code field at checkout even if you are outside the US and Canada.';
	$_REQUEST['key_help'] .= ' This is usually defined by the checkout.js javascript when the user selects their country.';
	$_Config->addEntry();
	$_DB->execute("UPDATE `config` SET `sequence` = 11 WHERE `section` = 'basics' AND `key` = 'always_display_area_code'");
}


// This is an update to add the bypass_plugin_on_zero_weight variable into the shipping table
// and add some help for the other variables.
$shipCalcFields = $_DB->getFieldProperties('shipping');
if(!isset($shipCalcFields['bypass_plugin_on_zero_weight'])){
	$_DB->execute("ALTER TABLE `shipping` ADD `bypass_plugin_on_zero_weight` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'true' AFTER `shipping_plugin_name`");
	$_DB->execute("ALTER TABLE `shipping` ADD `offer_local_pickup` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'false'");
	$_DB->execute("ALTER TABLE `shipping` ADD `local_pickup_text` VARCHAR( 255 ) NOT NULL DEFAULT 'Local Pickup';");

	$helpEntries = array();
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', 'This function allows you to set shipping preferences and turn on the shipping plug-ins.','','')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'bypass_plugin_on_zero_weight', 'Allows you to bypass the shipping plug-ins and use the default logic when the weight in the cart is zero.')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'require_shipping', 'Allows you to turn on/off shipping.')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'hide_shipping_on_zero_weight', 'Allows you to turn off shipping when the weight in the cart is zero.')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'use_shipping_plugin', 'Allows you to turn on the shipping plug-ins.')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'shipping_plugin_name', 'This is the name or names of the shipping plug-ins you would like to use. If you enter a comma delimited list like: ups.standard.inc,usps.xml.inc then a pull-down box will appear on the checkout screen that allows the user to select their shipping preference and only those rates will be displayed.')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'bypass_plugin_on_zero_weight', 'Allows you to bypass the shipping plug-ins and use the default logic when the weight in the cart is zero.')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'free_shipping_text', 'Text displayed to the user when the shipping total is zero.')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'default_shipping_region', 'Your default shipping region.')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'offer_local_pickup', 'Offer Local Pick-up as an option for shipping?')";
	$helpEntries[] = "INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'local_pickup_text', 'The text displayed for the Local Pick-up shipping option.')";

	foreach($helpEntries as $i=>$sql){
		$_DB->execute($sql);
	}
}

if(!isset($shipCalcFields['use_decimal_weight'])){
	$_DB->execute("ALTER TABLE `shipping` ADD `use_decimal_weight` ENUM( 'true', 'false' ) NOT NULL DEFAULT 'false'");
	$_DB->execute("INSERT INTO `help` (`section`, `section_help`, `key`, `key_help`) VALUES ('shipping', '', 'use_decimal_weight', 'Use decimal weights for shipping plug-ins? ie. 1.4 lbs.')");
}


// update for insurance field
$orderFlds = $_DB->getFieldProperties('orders');
if(!isset($orderFlds['insurance'])){
	$_DB->execute("ALTER TABLE `orders` ADD `insurance` FLOAT(10,2) NOT NULL DEFAULT '0.00' AFTER `shipping`");
}
if(isset($orderFlds['tracking_number'][1]) && $orderFlds['tracking_number'][1] != 'text'){
	$_DB->execute("ALTER TABLE `orders` CHANGE `tracking_number` `tracking_number` TEXT");
}
if(!isset($orderFlds['number_of_packages'])){
	$_DB->execute("ALTER TABLE `orders` ADD `number_of_packages` INT( 10 ) NOT NULL DEFAULT '1' AFTER `shipping_method`");
}
if(isset($orderFlds['tracking_number'][1]) && $orderFlds['tracking_number'][1] != 'varchar(255)'){
	$_DB->execute("ALTER TABLE `orders` CHANGE `transaction_status` `transaction_status` VARCHAR( 255 ) NULL");
}


?>