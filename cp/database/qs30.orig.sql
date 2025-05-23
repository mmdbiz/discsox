# phpMyAdmin MySQL-Dump
# version 2.5.1
# http://www.phpmyadmin.net/ (download page)
#
# Host: localhost
# Generation Time: Mar 20, 2007 at 12:28 PM
# Server version: 4.0.21
# PHP Version: 5.1.2
# Database : `qs30`
# --------------------------------------------------------

#
# Table structure for table `abandoned_carts`
#
# Creation: Jan 18, 2007 at 09:25 AM
# Last update: Mar 16, 2007 at 04:53 PM
#

CREATE TABLE `abandoned_carts` (
  `caid` int(10) NOT NULL auto_increment,
  `sid` varchar(32) NOT NULL default '',
  `date` date NOT NULL default '0000-00-00',
  `number_of_items` int(10) NOT NULL default '0',
  `cart_total` float(10,2) NOT NULL default '0.00',
  `last_page` varchar(255) NOT NULL default '',
  `email_address` varchar(50) NOT NULL default '',
  `username` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`caid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `buy_buttons`
#
# Creation: Apr 13, 2006 at 04:24 PM
# Last update: Apr 13, 2006 at 03:24 PM
# Last check: Apr 20, 2006 at 04:32 PM
#

CREATE TABLE `buy_buttons` (
  `bbid` int(10) NOT NULL auto_increment,
  `scripturl` varchar(255) NOT NULL default 'cart.php',
  `buybuttontext` varchar(50) NOT NULL default 'Buy Now',
  `viewbuttontext` varchar(50) NOT NULL default 'Review Cart',
  `qtytext` varchar(255) NOT NULL default 'Quantity',
  `add_image` varchar(255) NOT NULL default '',
  `view_image` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`bbid`)
) TYPE=MyISAM COMMENT='Defaults for Buy Buttons' AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `calculation_sequence`
#
# Creation: Mar 20, 2006 at 12:08 PM
# Last update: Mar 20, 2006 at 12:08 PM
# Last check: Apr 20, 2006 at 04:32 PM
#

CREATE TABLE `calculation_sequence` (
  `csid` int(10) NOT NULL auto_increment,
  `calculate_discount` enum('0','1','2','3') NOT NULL default '1',
  `calculate_sales_tax` enum('0','1','2','3') NOT NULL default '1',
  `calculate_shipping` enum('0','1','2','3') NOT NULL default '1',
  PRIMARY KEY  (`csid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `cart_details`
#
# Creation: Jan 18, 2007 at 07:09 PM
# Last update: Mar 16, 2007 at 04:53 PM
# Last check: Mar 16, 2007 at 04:53 PM
#

CREATE TABLE `cart_details` (
  `cdid` int(10) NOT NULL auto_increment,
  `cartid` int(10) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `price` varchar(255) NOT NULL default '0',
  `value` text NOT NULL,
  `weight` varchar(255) NOT NULL default '',
  `type` enum('option','setup') NOT NULL default 'option',
  PRIMARY KEY  (`cdid`),
  KEY `cartid` (`cartid`)
) TYPE=MyISAM COMMENT='Stores selected options for items in the cart' AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `carts`
#
# Creation: Jan 18, 2007 at 07:09 PM
# Last update: Mar 16, 2007 at 04:53 PM
# Last check: Mar 16, 2007 at 04:53 PM
#

CREATE TABLE `carts` (
  `cartid` int(10) NOT NULL auto_increment,
  `sessionid` varchar(32) NOT NULL default '',
  `page` varchar(255) NOT NULL default '',
  `shipping` varchar(255) NOT NULL default '0',
  `taxable` enum('true','false') NOT NULL default 'true',
  `tax_rate` float(5,4) NOT NULL default '0.0000',
  `tax_level` int(1) NOT NULL default '0',
  `quantity` float(10,2) NOT NULL default '0.00',
  `sku` varchar(50) NOT NULL default '',
  `name` varchar(255) NOT NULL default '',
  `price` varchar(255) NOT NULL default '0.00',
  `size` varchar(255) NOT NULL default '',
  `weight` varchar(255) NOT NULL default '',
  `download_filename` varchar(255) NOT NULL default '',
  `last_page` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`cartid`),
  KEY `sessionid` (`sessionid`)
) TYPE=MyISAM COMMENT='Maintains selected items for the cart' AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `categories`
#
# Creation: Mar 13, 2007 at 11:23 AM
# Last update: Mar 13, 2007 at 11:43 AM
#

CREATE TABLE `categories` (
  `catid` int(10) unsigned NOT NULL auto_increment,
  `parentid` int(10) unsigned NOT NULL default '0',
  `category_name` varchar(255) NOT NULL default '',
  `category_description` text NOT NULL,
  `category_discount` varchar(10) NOT NULL default '0%',
  `category_thumbnail` varchar(255) NOT NULL default '',
  `category_link` varchar(255) NOT NULL default '',
  `category_ids` varchar(50) NOT NULL default '',
  `display_category` enum('true','false') NOT NULL default 'true',
  PRIMARY KEY  (`catid`),
  KEY `parentid` (`parentid`),
  KEY `category_ids` (`category_ids`),
  FULLTEXT KEY `category_name,category_link` (`category_name`,`category_link`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `category_menu`
#
# Creation: Mar 20, 2007 at 11:26 AM
# Last update: Mar 20, 2007 at 11:26 AM
#

CREATE TABLE `category_menu` (
  `category_list` longblob NOT NULL,
  `timestamp` time default NULL
) TYPE=MyISAM;
# --------------------------------------------------------

#
# Table structure for table `config`
#
# Creation: Apr 13, 2006 at 05:30 PM
# Last update: Mar 13, 2007 at 02:43 PM
# Last check: Jan 09, 2007 at 09:58 AM
#

CREATE TABLE `config` (
  `configid` int(10) unsigned NOT NULL auto_increment,
  `section` varchar(50) default NULL,
  `sequence` int(3) NOT NULL default '0',
  `key` varchar(50) default NULL,
  `value` text NOT NULL,
  PRIMARY KEY  (`configid`)
) TYPE=MyISAM PACK_KEYS=0 AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `credit_cards`
#
# Creation: Jan 08, 2007 at 05:03 PM
# Last update: Mar 13, 2007 at 11:43 AM
# Last check: Jan 08, 2007 at 06:02 PM
#

CREATE TABLE `credit_cards` (
  `ccid` int(10) NOT NULL auto_increment,
  `card_name` varchar(50) NOT NULL default '',
  `active` enum('true','false') NOT NULL default 'true',
  PRIMARY KEY  (`ccid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `customer_favorites`
#
# Creation: Jun 14, 2005 at 03:03 PM
# Last update: Nov 13, 2006 at 12:46 PM
# Last check: Nov 13, 2006 at 12:46 PM
#

CREATE TABLE `customer_favorites` (
  `cfid` int(10) unsigned NOT NULL auto_increment,
  `cid` int(10) unsigned NOT NULL default '0',
  `pid` int(10) unsigned NOT NULL default '0',
  `page` varchar(50) default NULL,
  PRIMARY KEY  (`cfid`),
  KEY `cfid` (`cfid`),
  KEY `cid` (`cid`),
  KEY `pid` (`pid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `customer_shipping`
#
# Creation: Mar 09, 2006 at 06:27 PM
# Last update: Nov 10, 2006 at 09:36 AM
# Last check: Nov 10, 2006 at 09:36 AM
#

CREATE TABLE `customer_shipping` (
  `csid` int(10) unsigned NOT NULL auto_increment,
  `cid` int(10) unsigned NOT NULL default '0',
  `primary_address` enum('true','false') NOT NULL default 'false',
  `shipaddress_companyname` varchar(50) NOT NULL default '',
  `shipaddress_firstname` varchar(50) NOT NULL default '',
  `shipaddress_lastname` varchar(50) NOT NULL default '',
  `shipaddress_addr1` varchar(50) NOT NULL default '',
  `shipaddress_addr2` varchar(50) NOT NULL default '',
  `shipaddress_city` varchar(50) NOT NULL default '',
  `shipaddress_county` varchar(50) NOT NULL default '',
  `shipaddress_state` varchar(50) NOT NULL default '',
  `shipaddress_postalcode` varchar(50) NOT NULL default '',
  `shipaddress_country` varchar(50) NOT NULL default '',
  `shipaddress_areacode` char(3) NOT NULL default '000',
  `shipaddress_phone` varchar(8) NOT NULL default '000-0000',
  `shipaddress_email` varchar(50) NOT NULL default '',
  `shipaddress_delivery_type` enum('residential','commercial') NOT NULL default 'residential',
  PRIMARY KEY  (`csid`),
  KEY `cid` (`cid`),
  KEY `shipaddress_postalcode` (`shipaddress_postalcode`)
) TYPE=MyISAM AUTO_INCREMENT=1000 ;
# --------------------------------------------------------

#
# Table structure for table `customers`
#
# Creation: Jun 27, 2006 at 01:44 PM
# Last update: Mar 13, 2007 at 04:13 PM
#

CREATE TABLE `customers` (
  `cid` int(10) unsigned NOT NULL auto_increment,
  `customer_number` int(10) NOT NULL default '1000',
  `active` enum('true','false') NOT NULL default 'true',
  `active_date` date NOT NULL default '0000-00-00',
  `customer_type` varchar(50) NOT NULL default 'retail',
  `discount_rate` varchar(50) NOT NULL default '0%',
  `discount_type` enum('percentage','subtotal','quantity') NOT NULL default 'percentage',
  `discount_text` varchar(255) NOT NULL default 'Customer Discount',
  `billaddress_companyname` varchar(255) default NULL,
  `billaddress_firstname` varchar(50) default NULL,
  `billaddress_lastname` varchar(50) default NULL,
  `billaddress_addr1` varchar(50) default NULL,
  `billaddress_addr2` varchar(50) default NULL,
  `billaddress_city` varchar(50) default NULL,
  `billaddress_state` varchar(50) default NULL,
  `billaddress_county` varchar(50) NOT NULL default '',
  `billaddress_postalcode` varchar(50) default NULL,
  `billaddress_country` varchar(50) default 'US',
  `billaddress_areacode` char(3) NOT NULL default '000',
  `billaddress_phone` varchar(8) NOT NULL default '000-0000',
  `billaddress_email` varchar(50) default NULL,
  `email_list` enum('true','false') NOT NULL default 'true',
  `cust_notes` text,
  `username` varchar(50) default NULL,
  `password` varchar(32) default NULL,
  `is_taxable` enum('true','false') NOT NULL default 'true',
  PRIMARY KEY  (`cid`),
  KEY `customer_number` (`customer_number`)
) TYPE=MyISAM AUTO_INCREMENT=1000 ;
# --------------------------------------------------------

#
# Table structure for table `discounts`
#
# Creation: Mar 20, 2006 at 12:08 PM
# Last update: Mar 20, 2006 at 12:21 PM
# Last check: Apr 20, 2006 at 04:32 PM
#

CREATE TABLE `discounts` (
  `did` int(10) NOT NULL auto_increment,
  `name` varchar(100) NOT NULL default '',
  `coupon` varchar(32) NOT NULL default '',
  `remove_after_use` enum('true','false') NOT NULL default 'false',
  `subtotal_ranges` text NOT NULL,
  `qty_ranges` text NOT NULL,
  `free_shipping` enum('true','false') NOT NULL default 'false',
  PRIMARY KEY  (`did`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `downloads`
#
# Creation: May 24, 2007 at 11:54 AM
# Last update: May 24, 2007 at 11:54 AM
#

CREATE TABLE `downloads` (
  `dlid` int(11) NOT NULL auto_increment,
  `order_number` varchar(25) NOT NULL default '',
  `email` varchar(50) NOT NULL default '',
  `filename` varchar(100) NOT NULL default '',
  `key` varchar(32) NOT NULL default '',
  `complete` enum('true','false') NOT NULL default 'false',
  PRIMARY KEY  (`dlid`)
) TYPE=MyISAM PACK_KEYS=0 AUTO_INCREMENT=1 ;


#
# Table structure for table `extensions`
#
# Creation: Nov 21, 2006 at 11:03 AM
# Last update: Jan 08, 2007 at 04:01 PM
# Last check: Jan 08, 2007 at 04:01 PM
#

CREATE TABLE `extensions` (
  `extid` int(10) NOT NULL auto_increment,
  `class_to_extend` varchar(255) NOT NULL default '',
  `extended_class_name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`extid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `help`
#
# Creation: Jan 09, 2007 at 05:31 PM
# Last update: Jan 18, 2007 at 02:48 PM
#

CREATE TABLE `help` (
  `hid` int(10) NOT NULL auto_increment,
  `section` varchar(50) NOT NULL default '',
  `section_help` text NOT NULL,
  `key` varchar(50) NOT NULL default '',
  `key_help` text NOT NULL,
  PRIMARY KEY  (`hid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `install`
#
# Creation: Mar 20, 2007 at 11:28 AM
# Last update: Mar 20, 2007 at 11:28 AM
#

CREATE TABLE `install` (
  `iid` int(10) NOT NULL auto_increment,
  `install_date` date NOT NULL default '0000-00-00',
  `file_name` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`iid`)
) TYPE=MyISAM COMMENT='Installation details' AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `inventory`
#
# Creation: Aug 30, 2006 at 05:36 PM
# Last update: Aug 30, 2006 at 04:36 PM
#

CREATE TABLE `inventory` (
  `iid` int(10) NOT NULL auto_increment,
  `pid` int(10) NOT NULL default '0',
  `oids` varchar(50) NOT NULL default '',
  `odids` varchar(50) NOT NULL default '',
  `name` varchar(255) NOT NULL default '',
  `minimum_quantity` int(10) NOT NULL default '0',
  `maximum_quantity` int(10) NOT NULL default '0',
  `quantity_available` int(10) NOT NULL default '0',
  `quantity_sold` int(10) NOT NULL default '0',
  PRIMARY KEY  (`iid`),
  KEY `pid` (`pid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `manufacturers`
#
# Creation: Nov 14, 2006 at 05:27 PM
# Last update: Mar 13, 2007 at 11:43 AM
# Last check: Feb 06, 2007 at 11:27 AM
#

CREATE TABLE `manufacturers` (
  `mid` int(10) NOT NULL auto_increment,
  `mfg_name` varchar(255) NOT NULL default '',
  `display_mfg` enum('true','false') NOT NULL default 'true',
  PRIMARY KEY  (`mid`)
) TYPE=MyISAM PACK_KEYS=0 AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `option_details`
#
# Creation: Mar 13, 2007 at 11:23 AM
# Last update: Mar 13, 2007 at 12:32 PM
#

CREATE TABLE `option_details` (
  `odid` int(10) unsigned NOT NULL auto_increment,
  `oid` int(10) unsigned NOT NULL default '0',
  `sequence` int(10) NOT NULL default '0',
  `value` varchar(255) default NULL,
  `price` float(10,2) default NULL,
  `weight` varchar(255) default NULL,
  `text` varchar(255) default NULL,
  PRIMARY KEY  (`odid`),
  KEY `oid` (`oid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `option_import`
#
# Creation: Mar 13, 2007 at 11:23 AM
# Last update: Mar 13, 2007 at 11:23 AM
#

CREATE TABLE `option_import` (
  `sku` varchar(50) NOT NULL default '',
  `number` varchar(50) default NULL,
  `format` char(2) default '1',
  `name` varchar(255) default NULL,
  `value` varchar(255) default NULL,
  `price` varchar(255) default NULL,
  `weight` varchar(50) default NULL,
  `text` varchar(255) default NULL,
  `required` enum('true','false') default 'true',
  KEY  (`sku`)
) TYPE=MyISAM;

# --------------------------------------------------------

#
# Table structure for table `options`
#
# Creation: Mar 13, 2007 at 11:23 AM
# Last update: Mar 13, 2007 at 12:43 PM
#

CREATE TABLE `options` (
  `oid` int(10) unsigned NOT NULL auto_increment,
  `number` varchar(50) default NULL,
  `name` varchar(125) default NULL,
  `description` varchar(255) default NULL,
  `format` enum('select box','radio buttons','text box') NOT NULL default 'select box',
  `type` enum('option','setup') NOT NULL default 'option',
  `required` enum('true','false') default 'false',
  PRIMARY KEY  (`oid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `order_details`
#
# Creation: Apr 04, 2006 at 12:07 PM
# Last update: Mar 13, 2007 at 02:43 PM
# Last check: Jul 24, 2006 at 12:57 PM
#

CREATE TABLE `order_details` (
  `ordid` int(10) unsigned NOT NULL auto_increment,
  `orid` int(10) unsigned NOT NULL default '0',
  `page` text NOT NULL,
  `shipping` varchar(255) NOT NULL default '0',
  `taxable` enum('true','false') NOT NULL default 'true',
  `tax_rate` float(5,4) NOT NULL default '0.0000',
  `tax_level` int(1) NOT NULL default '0',
  `quantity` int(10) NOT NULL default '0',
  `sku` varchar(50) NOT NULL default '',
  `name` varchar(255) NOT NULL default '',
  `price` text NOT NULL,
  `size` varchar(50) NOT NULL default '',
  `weight` varchar(50) NOT NULL default '',
  `download_filename` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`ordid`),
  KEY `orid` (`orid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `order_options`
#
# Creation: Apr 04, 2006 at 12:07 PM
# Last update: Mar 13, 2007 at 02:43 PM
# Last check: Jul 24, 2006 at 12:57 PM
#

CREATE TABLE `order_options` (
  `oroid` int(10) NOT NULL auto_increment,
  `ordid` int(10) NOT NULL default '0',
  `name` varchar(255) NOT NULL default '',
  `price` varchar(255) NOT NULL default '0',
  `value` text NOT NULL,
  `weight` varchar(255) NOT NULL default '',
  `type` enum('option','setup') NOT NULL default 'option',
  PRIMARY KEY  (`oroid`),
  KEY `ordid` (`ordid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `orders`
#
# Creation: Nov 09, 2006 at 05:11 PM
# Last update: Mar 13, 2007 at 02:43 PM
# Last check: Nov 09, 2006 at 05:11 PM
#

CREATE TABLE `orders` (
  `orid` int(10) unsigned NOT NULL auto_increment,
  `cid` int(10) unsigned default NULL,
  `csid` int(10) unsigned default NULL,
  `order_number` varchar(50) default NULL,
  `affiliate` varchar(50) default NULL,
  `coupon` varchar(50) default NULL,
  `transaction_status` varchar(50) NOT NULL default '',
  `transaction_date` date NOT NULL default '0000-00-00',
  `transaction_time` time NOT NULL default '00:00:00',
  `status` enum('Not Shipped','Partial Shipped','Complete','Cancelled') NOT NULL default 'Not Shipped',
  `shipping_method` varchar(50) NOT NULL default '',
  `tracking_number` varchar(50) default NULL,
  `date_shipped` date NOT NULL default '0000-00-00',
  `payment_method` varchar(50) default NULL,
  `authorization_code` varchar(50) NOT NULL default '',
  `reference_number` varchar(50) NOT NULL default '',
  `name_on_card` varchar(50) default NULL,
  `credit_card_type` varchar(50) default NULL,
  `card_number` blob,
  `cvv2` blob,
  `expire_month` char(2) NOT NULL default '',
  `expire_year` varchar(4) NOT NULL default '',
  `bank_name` varchar(50) default NULL,
  `name_on_account` varchar(50) default NULL,
  `account_number` varchar(50) default NULL,
  `aba_routing_code` varchar(50) default NULL,
  `po_number` varchar(50) default NULL,
  `tax_number` varchar(50) default NULL,
  `comments` text,
  `email_list` enum('true','false') NOT NULL default 'true',
  `user_host` varchar(50) default NULL,
  `browser` varchar(50) NOT NULL default '',
  `paid` enum('true','false') default 'false',
  `subtotal` float(10,2) NOT NULL default '0.00',
  `credit` float(10,2) NOT NULL default '0.00',
  `discount` float(10,2) NOT NULL default '0.00',
  `shipping` float(10,2) NOT NULL default '0.00',
  `salestax` float(10,2) NOT NULL default '0.00',
  `handling` float(10,2) NOT NULL default '0.00',
  `gst` float(10,2) NOT NULL default '0.00',
  `hst` float(10,2) NOT NULL default '0.00',
  `pst` float(10,2) NOT NULL default '0.00',
  `grandtotal` float(10,2) NOT NULL default '0.00',
  PRIMARY KEY  (`orid`),
  KEY `cid` (`cid`),
  KEY `csid` (`csid`),
  KEY `order_number` (`order_number`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `payment_gateway_details`
#
# Creation: Jan 09, 2007 at 04:11 PM
# Last update: Mar 13, 2007 at 03:43 PM
# Last check: Jan 09, 2007 at 04:11 PM
#

CREATE TABLE `payment_gateway_details` (
  `pgdid` int(10) NOT NULL auto_increment,
  `pgid` int(10) NOT NULL default '0',
  `section` varchar(50) default NULL,
  `sequence` int(3) NOT NULL default '0',
  `key` varchar(255) default NULL,
  `value` text NOT NULL,
  PRIMARY KEY  (`pgdid`),
  KEY `pgid` (`pgid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `payment_gateways`
#
# Creation: Jan 11, 2007 at 05:30 PM
# Last update: Jan 16, 2007 at 06:08 PM
#

CREATE TABLE `payment_gateways` (
  `pgid` int(10) NOT NULL auto_increment,
  `gateway_name` varchar(50) NOT NULL default '',
  `active` enum('true','false') NOT NULL default 'true',
  `debug` enum('true','false') NOT NULL default 'false',
  `related_payment_form` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`pgid`)
) TYPE=MyISAM PACK_KEYS=0 AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `payment_methods`
#
# Creation: Mar 13, 2007 at 11:26 AM
# Last update: Mar 13, 2007 at 11:43 AM
#

CREATE TABLE `payment_methods` (
  `pmid` int(10) NOT NULL auto_increment,
  `method` varchar(50) NOT NULL default '',
  `active` enum('true','false') NOT NULL default 'false',
  PRIMARY KEY  (`pmid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `paypal`
#
# Creation: Mar 28, 2006 at 06:07 PM
# Last update: Mar 13, 2007 at 02:29 PM
# Last check: Mar 13, 2007 at 02:29 PM
#

CREATE TABLE `paypal` (
  `ppid` varchar(32) NOT NULL default '0',
  `sid` varchar(32) NOT NULL default '',
  `payment_status` varchar(25) NOT NULL default '',
  `payment_gross` double(10,2) NOT NULL default '0.00',
  `payer_email` varchar(25) NOT NULL default '',
  `memo` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`ppid`)
) TYPE=MyISAM;
# --------------------------------------------------------

#
# Table structure for table `product_categories`
#
# Creation: Mar 13, 2007 at 11:23 AM
# Last update: Mar 16, 2007 at 03:43 PM
#

CREATE TABLE `product_categories` (
  `pcid` int(10) unsigned NOT NULL auto_increment,
  `pid` int(10) unsigned NOT NULL default '0',
  `catid` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`pcid`),
  KEY `pid` (`pid`),
  KEY `catid` (`catid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `product_custom`
#
# Creation: Mar 13, 2007 at 11:23 AM
# Last update: Mar 13, 2007 at 11:43 AM
# Last check: Mar 13, 2007 at 11:23 AM
#

CREATE TABLE `product_custom` (
  `customid` int(10) unsigned NOT NULL auto_increment,
  `custom_pid` int(10) unsigned NOT NULL default '0',
  `detail_page_title` text NOT NULL,
  `detail_meta_keywords` text NOT NULL,
  `detail_meta_description` text NOT NULL,
  PRIMARY KEY  (`customid`),
  KEY `custom_pid` (`custom_pid`)
) ENGINE=MyISAM AUTO_INCREMENT=1;


# --------------------------------------------------------
#
# Table structure for table `product_edit`
#
# Creation: May 08, 2006 at 12:05 PM
# Last update: Mar 13, 2007 at 12:43 PM
# Last check: Jul 24, 2006 at 12:57 PM
#

CREATE TABLE `product_edit` (
  `peid` int(10) NOT NULL auto_increment,
  `related_table` varchar(50) NOT NULL default '',
  `key` varchar(50) NOT NULL default '',
  `value` varchar(255) NOT NULL default '',
  PRIMARY KEY  (`peid`)
) TYPE=MyISAM COMMENT='Used for the product and option edit screens' AUTO_INCREMENT=1 ;

# --------------------------------------------------------
#
# Table structure for table `product_images`
#
CREATE TABLE `product_images` (
  `iid` int(10) NOT NULL auto_increment,
  `pid` int(10) NOT NULL,
  `thumbnail` varchar(255) NOT NULL,
  `thumbnail_alt_tag` varchar(255) NOT NULL,
  `fullsize` varchar(255) NOT NULL,
  `fullsize_alt_tag` varchar(255) NOT NULL,
  PRIMARY KEY  (`iid`),
  KEY `pid` (`pid`)
) ENGINE=MyISAM  AUTO_INCREMENT=1 ;


# --------------------------------------------------------
#
# Table structure for table `product_import`
#
# Creation: Mar 13, 2007 at 11:23 AM
# Last update: Mar 13, 2007 at 11:23 AM
#

CREATE TABLE `product_import` (
  `sku` varchar(50) NOT NULL default '',
  `category` varchar(255) NOT NULL default '',
  `price` varchar(255) NOT NULL default '',
  `name` varchar(255) NOT NULL default '',
  `description` text NOT NULL,
  `fullsize_image` varchar(50) NOT NULL default '',
  `thumbnail_image` varchar(50) NOT NULL default '',
  `size` varchar(255) NOT NULL default '',
  `weight` varchar(50) NOT NULL default '',
  `link_page` varchar(255) NOT NULL default '',
  `link_text` varchar(255) NOT NULL default '',
  `user_1` varchar(255) NOT NULL default '',
  `user_2` varchar(255) NOT NULL default '',
  `user_3` varchar(255) NOT NULL default '',
  `user_4` varchar(255) NOT NULL default '',
  `options` varchar(50) NOT NULL default '',
  PRIMARY KEY  (`sku`)
) TYPE=MyISAM;
# --------------------------------------------------------

#
# Table structure for table `product_options`
#
# Creation: Mar 13, 2007 at 11:23 AM
# Last update: Mar 13, 2007 at 12:43 PM
# Last check: Mar 13, 2007 at 12:31 PM
#

CREATE TABLE `product_options` (
  `poid` int(10) unsigned NOT NULL auto_increment,
  `pid` int(10) unsigned NOT NULL default '0',
  `oid` int(10) unsigned NOT NULL default '0',
  `sequence` int(3) NOT NULL default '0',
  PRIMARY KEY  (`poid`),
  KEY `poid` (`poid`),
  KEY `pid` (`pid`),
  KEY `oid` (`oid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `product_shipping`
#
# Creation: Apr 19, 2006 at 02:44 PM
# Last update: Apr 19, 2006 at 01:44 PM
# Last check: Apr 20, 2006 at 04:32 PM
#

CREATE TABLE `product_shipping` (
  `psid` int(10) unsigned NOT NULL auto_increment,
  `pid` int(10) NOT NULL default '0',
  `packaging` varchar(100) default NULL,
  `qty_per_box` int(10) default NULL,
  `lbs` int(10) NOT NULL default '0',
  `ounces` enum('0','1','2','3','4','5','6','7','8','9','10','11','12','13','14','15') NOT NULL default '0',
  `origin_country` varchar(100) default NULL,
  `origin_city` varchar(100) default NULL,
  `origin_zip` int(10) default NULL,
  `flat_rate` double(10,2) default NULL,
  `flat_rate_unit` enum('any quantity','each') default NULL,
  PRIMARY KEY  (`psid`),
  KEY `sku` (`pid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `products`
#
# Creation: Mar 13, 2007 at 11:23 AM
# Last update: Mar 16, 2007 at 03:43 PM
#

CREATE TABLE `products` (
  `pid` int(10) unsigned NOT NULL auto_increment,
  `sku` varchar(50) default NULL,
  `mid` int(10) NOT NULL default '1',
  `display_product` enum('true','false') NOT NULL default 'true',
  `on_sale` enum('true','false') NOT NULL default 'false',
  `retail_price` double(10,2) default NULL,
  `name` varchar(255) default NULL,
  `description` text,
  `price` tinytext,
  `color` varchar(50) default NULL,
  `size` varchar(50) default NULL,
  `weight` varchar(50) default NULL,
  `link_page` varchar(50) default NULL,
  `link_text` varchar(50) default NULL,
  `thumbnail_image` varchar(50) default NULL,
  `fullsize_image` varchar(50) default NULL,
  `is_taxable` enum('true','false') NOT NULL default 'true',
  `tax_rate` float(5,4) NOT NULL default '0.0000',
  `inventory_item` enum('true','false') NOT NULL default 'false',
  `inventory_options` enum('true','false') NOT NULL default 'false',
  `display_when_sold_out` enum('true','false') NOT NULL default 'false',
  `is_downloadable` enum('true','false') NOT NULL default 'false',
  `download_filename` varchar(255) NOT NULL default '',
  `page` varchar(255) NOT NULL default '',
  `last_modified` date NOT NULL default '0000-00-00',
  PRIMARY KEY  (`pid`),
  KEY `sku` (`sku`),
  FULLTEXT KEY `name,description` (`name`,`description`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `related_products`
#
# Creation: Apr 19, 2006 at 02:43 PM
# Last update: Apr 19, 2006 at 01:43 PM
#

CREATE TABLE `related_products` (
  `rpid` int(10) NOT NULL auto_increment,
  `pid` int(10) NOT NULL default '0',
  `related_pid` int(10) NOT NULL default '0',
  PRIMARY KEY  (`rpid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `sales_tax`
#
# Creation: Apr 13, 2006 at 05:21 PM
# Last update: Apr 13, 2006 at 04:21 PM
# Last check: Apr 20, 2006 at 04:32 PM
#

CREATE TABLE `sales_tax` (
  `stid` int(10) NOT NULL auto_increment,
  `calculate_us_tax` enum('true','false') NOT NULL default 'true',
  `calculate_ca_tax` enum('true','false') NOT NULL default 'false',
  `calculate_vat_tax` enum('true','false') NOT NULL default 'false',
  `apply_gst_and_hst_to_shipping` enum('true','false') NOT NULL default 'true',
  `section_help` text NOT NULL,
  PRIMARY KEY  (`stid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `sales_tax_ca`
#
# Creation: Mar 20, 2006 at 12:11 PM
# Last update: Mar 20, 2006 at 12:11 PM
# Last check: Apr 20, 2006 at 04:32 PM
#

CREATE TABLE `sales_tax_ca` (
  `stcaid` int(10) unsigned NOT NULL auto_increment,
  `province` varchar(50) default NULL,
  `gst` double(10,4) NOT NULL default '0.0000',
  `pst` double(10,4) NOT NULL default '0.0000',
  `hst` double(10,4) NOT NULL default '0.0000',
  PRIMARY KEY  (`stcaid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `sales_tax_us`
#
# Creation: Mar 20, 2006 at 12:11 PM
# Last update: Mar 20, 2006 at 12:11 PM
# Last check: Apr 20, 2006 at 04:32 PM
#

CREATE TABLE `sales_tax_us` (
  `stusid` int(10) NOT NULL auto_increment,
  `state` char(2) NOT NULL default '',
  `rate` double(5,4) NOT NULL default '0.0000',
  `use_tax_table` enum('true','false') NOT NULL default 'false',
  `tax_table_form_field` enum('shipaddress_county','shipaddress_postalcode') NOT NULL default 'shipaddress_county',
  PRIMARY KEY  (`stusid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `sales_tax_vat`
#
# Creation: Mar 20, 2006 at 12:14 PM
# Last update: Mar 20, 2006 at 12:14 PM
# Last check: Apr 20, 2006 at 04:32 PM
#

CREATE TABLE `sales_tax_vat` (
  `stvatid` int(10) NOT NULL auto_increment,
  `country` varchar(50) NOT NULL default '',
  `rate` double(5,4) NOT NULL default '0.0000',
  PRIMARY KEY  (`stvatid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `sessions`
#
# Creation: Jan 18, 2007 at 07:09 PM
# Last update: Mar 20, 2007 at 11:27 AM
# Last check: Mar 20, 2007 at 11:16 AM
#

CREATE TABLE `sessions` (
  `ses_id` varchar(32) NOT NULL default '',
  `ses_time` int(11) NOT NULL default '0',
  `ses_start` int(11) NOT NULL default '0',
  `ses_fingerprint` varchar(32) NOT NULL default '',
  `ses_value` text NOT NULL,
  PRIMARY KEY  (`ses_id`)
) TYPE=MyISAM;
# --------------------------------------------------------

#
# Table structure for table `shipping`
#
# Creation: Apr 13, 2006 at 05:34 PM
# Last update: Nov 21, 2006 at 03:38 PM
#

CREATE TABLE `shipping` (
  `shid` int(10) NOT NULL auto_increment,
  `require_shipping` enum('true','false') NOT NULL default 'true',
  `hide_shipping_on_zero_weight` enum('true','false') NOT NULL default 'true',
  `use_shipping_plugin` enum('true','false') NOT NULL default 'false',
  `shipping_plugin_name` varchar(255) NOT NULL default '',
  `free_shipping_text` varchar(50) NOT NULL default 'Free Shipping',
  `default_shipping_region` varchar(5) NOT NULL default 'US',
  PRIMARY KEY  (`shid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `shipping_rates`
#
# Creation: Mar 20, 2006 at 12:17 PM
# Last update: Jan 09, 2007 at 02:32 PM
# Last check: Apr 20, 2006 at 04:32 PM
#

CREATE TABLE `shipping_rates` (
  `srid` int(10) NOT NULL auto_increment,
  `region` varchar(50) NOT NULL default '',
  `carrier` varchar(50) NOT NULL default '',
  `surcharge` text NOT NULL,
  `subtotal_range` text NOT NULL,
  `quantity_range` text NOT NULL,
  `weight_range` text NOT NULL,
  `handling` varchar(50) NOT NULL default '',
  PRIMARY KEY  (`srid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;
# --------------------------------------------------------

#
# Table structure for table `users`
#
# Creation: Apr 12, 2006 at 05:09 PM
# Last update: Nov 21, 2006 at 05:28 PM
# Last check: Jul 24, 2006 at 12:57 PM
#

CREATE TABLE `users` (
  `uid` int(10) unsigned NOT NULL auto_increment,
  `welcome_name` varchar(50) NOT NULL default '',
  `username` varchar(50) NOT NULL default '',
  `password` varchar(50) NOT NULL default '',
  `title` varchar(50) NOT NULL default '',
  `email_address` varchar(50) NOT NULL default '',
  `rights` text NOT NULL,
  PRIMARY KEY  (`uid`),
  KEY `uid` (`uid`)
) TYPE=MyISAM AUTO_INCREMENT=1 ;

