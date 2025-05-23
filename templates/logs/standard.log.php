
order number:		       <?=$order_number;?><?=$_CR;?>
order date:			       <?=$order_date;?><?=$_CR;?>
customer number:	       <?=$customer_number;?><?=$_CR;?>
user host ip:		       <?=$hostip;?><?=$_CR;?>
shipping method:	       <?=$_SESSION['shipping_method'];?><?=$_CR;?>
comments:			       <?=$_SESSION['comments'];?><?=$_CR;?>

<?php foreach($billingFields as $key=>$value):?>
<?=str_pad($key . ':',26);?> <?=$value;?><?=$_CR;?>
<?php endforeach;?>

<?php foreach($shippingFields as $key=>$value):?>
<?=str_pad($key . ':',26);?> <?=$value;?><?=$_CR;?>
<?php endforeach;?>

<?php foreach($paymentFields as $key=>$val):?>
<?=str_pad($key . ':',26);?> <?=$val;?><?=$_CR;?>
<?php endforeach;?>

<?php foreach($cart->items as $i=>$fields):?>
----------------------
item:		<?=$fields['product_id'];?><?=$_CR;?>
name:		<?=$fields['name'];?><?=$_CR;?>
<?php if(isset($fields['options'])):?>
<?php foreach($fields['options'] as $j=>$option):?>
<?php if(strtolower($option['name']) == 'option'):?>
<?php if($option['price'] > 0):?>
<?=$option['name'];?> <?=$j + 1;?>:		<?=$option['value'];?> (<?=number_format($option['price'],2);?>)<?=$_CR;?>
<?php else:?>
<?=$option['name'];?> <?=$j + 1;?>:		<?=$option['value'];?><?=$_CR;?>
<?php endif;?>
<?php else:?>
<?php if($option['price'] > 0):?>
<?=$option['name'];?>:		<?=$option['value'];?> (<?=number_format($option['price'],2);?>)<?=$_CR;?>
<?php else:?>
<?=$option['name'];?>:		<?=$option['value'];?><?=$_CR;?>
<?php endif;?>
<?php endif;?>
<?php endforeach;?>
<?php endif;?>
size:		<?=$fields['size'];?><?=$_CR;?>
weight:		<?=$fields['weight'];?><?=$_CR;?>
price:		<?=$_Common->format_price($fields['line_total'] / intval($fields['quantity']));?><?=$_CR;?>
quantity:	<?=intval($fields['quantity']);?><?=$_CR;?>
total:		<?=$fields['line_total'];?><?=$_CR;?>

<?php endforeach;?>
----------------------
subtotal:	<?=str_pad($cart->totals['subtotal'],10,' ',STR_PAD_LEFT);?><?=$_CR;?>
discount:	<?=str_pad($cart->totals['discount'],10,' ',STR_PAD_LEFT);?><?=$_CR;?>
salestax:	<?=str_pad($cart->totals['salestax'],10,' ',STR_PAD_LEFT);?><?=$_CR;?>
shipping:   <?=str_pad($cart->totals['shipping'],10,' ',STR_PAD_LEFT);?><?=$_CR;?>
grandtotal: <?=str_pad($cart->totals['grandtotal'],10,' ',STR_PAD_LEFT);?><?=$_CR;?>

