<Order>

<OrderDate><?=$_SESSION['order_date'];?></OrderDate>
<CustomerNumber><?=$_SESSION['customer_number'];?></CustomerNumber>
<OrderNumber><?=$_SESSION['order_number'];?></OrderNumber>
<UserHostIp><?=$_SESSION['user_host'];?></UserHostIp>
<Coupon><?=$_SESSION['coupon'];?></Coupon>

<BillAddress>
    <Company><?=$_SESSION['billaddress_companyname'];?></Company>
    <Name><?=$_SESSION['billaddress_firstname'];?> <?=$_SESSION['billaddress_lastname'];?></Name>
    <Addr1><?=$_SESSION['billaddress_addr1'];?></Addr1>
    <Addr2><?=$_SESSION['billaddress_addr2'];?></Addr2>
    <City><?=$_SESSION['billaddress_city'];?></City>
    <State><?=$_SESSION['billaddress_state'];?></State>
    <County><?=$_SESSION['billaddress_county'];?></County>
    <PostalCode><?=$_SESSION['billaddress_postalcode'];?></PostalCode>
    <Country><?=$_SESSION['billaddress_country'];?></Country>
    <Phone><?=$_SESSION['billaddress_areacode'];?>-<?=$_SESSION['billaddress_phone'];?></Phone>
    <Email><?=$_SESSION['billaddress_email'];?></Email>
</BillAddress>

<ShipAddress>
	<Company><?=$_SESSION['shipaddress_companyname'];?></Company>
    <Name><?=$_SESSION['shipaddress_firstname'];?> <?=$_SESSION['shipaddress_lastname'];?></Name>
    <Addr1><?=$_SESSION['shipaddress_addr1'];?></Addr1>
    <Addr2><?=$_SESSION['shipaddress_addr2'];?></Addr2>
    <City><?=$_SESSION['shipaddress_city'];?></City>
    <State><?=$_SESSION['shipaddress_state'];?></State>
    <County><?=$_SESSION['shipaddress_county'];?></County>
    <PostalCode><?=$_SESSION['shipaddress_postalcode'];?></PostalCode>
    <Country><?=$_SESSION['shipaddress_country'];?></Country>
    <Phone><?=$_SESSION['shipaddress_areacode'];?>-<?=$_SESSION['shipaddress_phone'];?></Phone>
    <Email><?=$_SESSION['shipaddress_email'];?></Email>
    <ShippingMethod><?=$_SESSION['shipping_method'];?></ShippingMethod>
    <DeliveryType><?=$_SESSION['shipaddress_delivery_type'];?></DeliveryType>
</ShipAddress>
<?php if(isset($_SESSION['payment_method'])):?>

<?php if(strstr($_SESSION['payment_method'],"credit_card")):?>
<Payment>
    <NameOnCard><?=$_SESSION['name_on_card'];?></NameOnCard>
    <CardType><?=$_SESSION['credit_card_type'];?></CardType>
    <CardNumber><?=$_SESSION['card_number'];?></CardNumber>
    <ExpireDate><?=$_SESSION['expire_month'];?>/<?=$_SESSION['expire_year'];?></ExpireDate>
    <VerificationNumber><?=$_SESSION['cvv2'];?></VerificationNumber>
</Payment>
<?php elseif(strstr($_SESSION['payment_method'],"check")):?>
<Payment>
	<BankName><?=$_SESSION['bank_name'];?></BankName>
	<AccountNumber><?=$_SESSION['account_number'];?></AccountNumber>
	<NameOnAccount><?=$_SESSION['name_on_account'];?></NameOnAccount>
	<ABARoutingCode><?=$_SESSION['aba_routing_code'];?></ABARoutingCode>
</Payment>
<?php elseif(strstr($_SESSION['payment_method'],"purchase_order")):?>
<Payment>
	<PONumber><?=$_SESSION['po_number'];?></PONumber>
</Payment>
<?php endif;?>
<?php endif;?>

<LineItems>
<?php foreach($_CART as $i=>$fields):?>
    <lineItem>
        <Item><?=$fields['sku'];?></Item>
        <Product><?=htmlspecialchars($fields['name'],ENT_QUOTES,'ISO-8859-1');?></Product>
        <?php if(isset($fields['options'])):?>
        <Options>
		<?php foreach($fields['options'] as $j=>$option):?>
	<Option>
		<?php if(strtolower($option['name']) == 'option'):?>
		<Name><?=htmlspecialchars($option['name'],ENT_QUOTES,'ISO-8859-1');?> <?=$j + 1;?></Name>
		<?php else:?>
		<Name><?=htmlspecialchars($option['name'],ENT_QUOTES,'ISO-8859-1');?></Name>
		<?php endif;?>
		<Value><?=htmlspecialchars($option['value'],ENT_QUOTES,'ISO-8859-1');?></Value>
				<Price><?=number_format($option['price'],2);?></Price>
			</Option>
		<?php endforeach;?>
</Options>
        <?php endif;?>
		<Size><?=$fields['size'];?></Size>
        <Weight><?=$fields['weight'];?></Weight>
        <Price><?=$_Common->format_price($fields['line_total'] / intval($fields['quantity']));?></Price>
        <Qty><?=intval($fields['quantity']);?></Qty>
        <Total><?=$fields['line_total'];?></Total>
    </lineItem>
<?php endforeach;?>
</LineItems>

<OrderTotals>
    <Subtotal><?=$subtotal;?></Subtotal>
    <Discount><?=$discount;?></Discount>
    <SalesTax><?=$salestax;?></SalesTax>
    <Shipping><?=$shipping;?></Shipping>
    <GrandTotal><?=$grandtotal;?></GrandTotal>
</OrderTotals>

<Comments><?=$_SESSION['comments'];?></Comments>

</Order>
