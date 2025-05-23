<?php

// this is an array of the fields, in order, 
// that will be saved to the csv file for EACH
// line in the cart. The cart fields are appended
// to the end of this list for each line.

$fields = array("order_number"				=> $order_number,
				"customer_number"			=> $customer_number,
				"order_date"				=> $order_date,
				"host_ip"					=> $hostip,
				"coupon"					=> $_SESSION['coupon'],
				"billaddress_companyname"	=> $_SESSION['billaddress_companyname'],
				"billaddress_firstname"		=> $_SESSION['billaddress_firstname'],
				"billaddress_lastname"		=> $_SESSION['billaddress_lastname'],
				"billaddress_addr1"			=> $_SESSION['billaddress_addr1'],
				"billaddress_addr2"			=> $_SESSION['billaddress_addr2'],
				"billaddress_city"			=> $_SESSION['billaddress_city'],
				"billaddress_state"			=> $_SESSION['billaddress_state'],
				"billaddress_county"		=> $_SESSION['billaddress_county'],
				"billaddress_postalcode"	=> $_SESSION['billaddress_postalcode'],
				"billaddress_country"		=> $_SESSION['billaddress_country'],
				"billaddress_phone"			=> $_SESSION['billaddress_area_code'] . '-' . $_SESSION['billaddress_phone'],
				"billaddress_email"			=> $_SESSION['billaddress_email'],
				"shipaddress_companyname"	=> $_SESSION['shipaddress_companyname'],
				"shipaddress_firstname"		=> $_SESSION['shipaddress_firstname'],
				"shipaddress_lastname"		=> $_SESSION['shipaddress_lastname'],
				"shipaddress_addr1"			=> $_SESSION['shipaddress_addr1'],
				"shipaddress_addr2"			=> $_SESSION['shipaddress_addr2'],
				"shipaddress_city"			=> $_SESSION['shipaddress_city'],
				"shipaddress_state"			=> $_SESSION['shipaddress_state'],
				"shipaddress_county"		=> $_SESSION['shipaddress_county'],
				"shipaddress_postalcode"	=> $_SESSION['shipaddress_postalcode'],
				"shipaddress_country"		=> $_SESSION['shipaddress_country'],
				"shipaddress_phone"			=> $_SESSION['shipaddress_area_code'] . '-' . $_SESSION['shipaddress_phone'],
				"shipaddress_email"			=> $_SESSION['shipaddress_email'],
				"shipaddress_delivery_type"	=> $_SESSION['shipaddress_delivery_type'],
				"shipping_method"			=> $_SESSION['shipping_method'],
				"payment_method"			=> $_SESSION['payment_method'],
				"name_on_card"				=> $_SESSION['name_on_card'],
				"credit_card_type"			=> $_SESSION['credit_card_type'],
				"card_number"				=> $_SESSION['card_number'],
				"expire_date"				=> $_SESSION['expire_month'] . '/' . $_SESSION['expire_year'],
				"cvv2"						=> $_SESSION['cvv2'],
				"bank_name"					=> $_SESSION['bank_name'],
				"account_number"			=> $_SESSION['account_number'],
				"name_on_account"			=> $_SESSION['name_on_account'],
				"aba_routing_code"			=> $_SESSION['aba_routing_code'],
				"po_number"					=> $_SESSION['po_number'],
				"subtotal"					=> $subtotal,
				"discount"					=> $discount,
				"salestax"					=> $salestax,
				"shipping"					=> $shipping,
				"grandtotal"				=> $grandtotal,
				"comments"					=> $_SESSION['comments']);

$headers = array_keys($fields);

// add cart fields
$headers[] = 'sku';
$headers[] = 'name';
$headers[] = 'options';
$headers[] = 'size';
$headers[] = 'weight';
$headers[] = 'price';
$headers[] = 'quantity';
$headers[] = 'line_total';

print '"' . join('","',$headers) . '"' . "\n";

foreach($_CART as $i=>$row){

	$csvFlds = array_values($fields);

	$csvFlds[] = $row['sku'];
	$csvFlds[] = $row['name'];

	$options = array();
	$optionTotal = 0;
	foreach($row['options'] as $i=>$option){
		$options[] = $option['value'];
		if($option['price'] > 0){
			$optionTotal += $option['price'];
		}
	}
	$csvFlds[] = join(', ',$options);
	$csvFlds[] = $row['size'];
	$csvFlds[] = $row['weight'];
	$csvFlds[] = $row['price'];
	$csvFlds[] = $row['quantity'];
	$csvFlds[] = number_format(($row['price'] + $optionTotal) * $row['quantity'],2);

	print '"' . join('","',$csvFlds) . '"' . "\n";
}
?>