<?php
// This is an update to add the bypass_plugin_on_zero_weight variable into the shipping table
// and add some help for the other variables.
global $_DB;

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
?>