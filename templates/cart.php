<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
<head>
<title>Your Shopping Cart</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">
<link rel="stylesheet" type="text/css" href="../../_css/bootstrap.css" />
<link type="text/css" rel="stylesheet" href="../../_css/responsive.css">
<link type="text/css" rel="stylesheet" href="../../_css/nav.css">
<link type="text/css" rel="stylesheet" href="../../_css/general.css">
<link type="text/css" rel="stylesheet" href="../../_css/product.css">
<link rel="stylesheet" type="text/css" href="../../_css/cart.css" />
</head>
<body>
<!--webbot bot="PurpleText" PREVIEW="
This page contains PHP script variables in the HTML that may be hidden in your editor.
So, please be careful editing this page and be sure to keep a backup copy before overwriting it.
View the HTML source code for more details.
"-->
<?php

				error_reporting(0);
$showPrices = $_CF['cart']['show_prices'];
//marcello testing ship to billing
$_SESSION['ship_to_billing_addr'] = 1;
//echo 'device set to: ' . $_SESSION['DisplayDevice'];
?>
<?php if(count($_CART) == 0): ?>
    <div>
      <h2>Your Cart is empty</h2>    
        <div class="inlineBlock m-l-3">
           <!-- CHeck for being on localhost or live site and set link accordingly -->
           <a href="<?php 
                echo (strpos($_SERVER['HTTP_HOST'], 'localhost') !== false) 
                    ? $_SERVER['HTTP_HOST'] . '/../../' 
                    : $_CF['basics']['home_page_name']; 
            ?>">
          <!-- <a href="<?=$_CF['basics']['home_page_name'];?>"> -->
            <!--<div class="largeSubmitButtonImg inlineBlock arrowLeft left">&nbsp;</div>
            <div class="largeSubmitLink inlineBlock">Continue Shopping</div>-->
            <button class="btn btn-danger m-l-05" type="button">
                <span class="glyphicon glyphicon-triangle-left" aria-hidden="true"></span>
                Continue Shopping
            </button>
          </a>
        </div>
    </div>
<?php else:?>
<div class="max-width-1000 ">
  <h2 class="text-left m-l-3"> Shopping Cart    </h2>
  <div class="center relative m-t--2  m-b-1">
    <div class="submitButtonContainer inlineBlock m-l-3 left">
      <a href="<?=$_CF['basics']['home_page_name'];?>">
        <button class="btn btn-danger" type="button">
            <span class="glyphicon glyphicon-triangle-left" aria-hidden="true"></span>
            Continue Shopping
        </button>
      </a>
    </div>
    <div class="submitButtonContainer inlineBlock m-l-3 ">
	    <script type="text/javascript"> 
			function OpenWindow(thisURL) { 
			   var myWin = window.open(thisURL,"shipquote", 
				 "toolbar=no,scrollbars=yes,resizable=yes,width=520,height=500"); 
				 return false; 
			} 
		  </script>
      <a href="#" onClick="return OpenWindow('shipping.quote.php');">
        <button class="btn btn-lightRed" type="button">
          Estimate Shipping
        </button>
      </a>
                          <!--<a rel="modal" href="../Support/shipping-quote.php" class="p-t-1"><strong><em>Shiping Quote</em></strong></a>-->
    </div>
    <div class="m-t--3 inlineBlock pull-right subTotalCheckoutContainer">
        <div class="inlineBlock pull-right alert alert-mmd m-b--2 p-b-1">
          <div class="subTotalCheckout m-r--2  m-b-1">
            <strong>Cart Subtotal:</strong> (<?=$miniCart['item_count'];?> item<?php if($miniCart['item_count'] >1):?>s<?php endif;?>)
            <strong><span class="red m-r-2"><?=$_Common->format_price($_Totals['subtotal'],true);?></span></strong>
          </div>
          
            <!-- CHeck for being on localhost or live site and set link accordingly -->
            <a href="<?php 
                echo (strpos($_SERVER['HTTP_HOST'], 'localhost') !== false) 
                    ? $_SERVER['HTTP_HOST'] . '/../checkout.php?cart_id=' . session_id() 
                    : $secure_url . '/checkout.php?cart_id=' . session_id(); 
            ?>">
            <button class="btn btn-danger m-l-05" type="button">
              Checkout
              <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span>
            </button>
          </a>
        </div>
    </div>
  </div>
</div>
<div class="max-width-1000 m-x-1">
  <form method="post" action="cart.php" ID="Form1" class="checkoutForm">
    <input type="hidden" name="page" value="<?=$_SESSION['last_page'];?>" ID="Hidden1">
    <input type="hidden" name="modify" value="1" ID="Hidden2">
    <table class="table table-striped table-condensed table-responsive  m-b-1" ID="Table2">
      <tr>
        <th class="cartHeader center"  >&nbsp;</th>
        <?php if($showPrices):?>
          <th class="cartHeader" >&nbsp;</th>
          <th class="cartHeader text-right" ><span class="m-r-1">Qty</span></th>
        <th class="cartHeader text-right"  ><span class=" m-r-2">Total</span></th>
        <?php else:?>
        <th class="text-right"  >Name</th>
        <?php endif;?>
      </tr>
      <!-- Start of each cart row -->
      <?php foreach($_CART as $i=>$fields): ?>
		<?php
			  $cartid = $fields['cartid'];
			  $lastItem = $fields['name'];
		?>
        <tr>
          <td class="cartRow" align="center">
            <?php 
                $thumb = $fields['thumbnail_image'];
                $thumbDir = $_CF['images']['thumbnail_images_directory'];
                $tWidth = $_CF['images']['product_thumbnail_max_width'];
                $tHeight = $_CF['images']['product_thumbnail_max_height'];
                $alt = "Image of " . $fields['name'];
                if(file_exists("$thumbDir/$thumb")){
                    $image = "<img src=\"$thumbDir/$thumb\" height=\"$tHeight\" width=\"$tWidth\" alt=\"$alt\">";
                    echo "<a href=\"" . $fields['page'] . "\">".$image ."</a>";
                }
            ?>
          </td>
          <td class="cartRow" align="left">
            <a href="<?=$fields['page'];?>"><strong>
            <?=$fields['name'];?>
            </strong></a> <span class="text-transparent small">sku: <?=$fields['sku'];?></span>
            <!-- Options -->
            <?php if(!empty($fields['options']) && is_array($fields['options'])):
                $kitDiscountAmount = 0;?>
                <br />
                <div class="xsmall m-l-1">
                  <?php foreach($fields['options'] as $j=>$option):?>
                    <div>
                      <?php if($option['name'] != "" && $option['name'] == "Option"):?>
                          <?=$option['name'];?><?=$j+1;?>:
                      <?php elseif($option['name'] != "" && $option['name'] != "Option"):?>
                          <?=$option['name'];?>:
                      <?php endif;?>
                      <?=$option['value'];?>
                      <?php if($showPrices):?>
                          <?php if($option['price'] != "0"):?>
                              <?php if($option['type'] == "option"):?>
                                  ($<?=$option['price'];?>)
                              <?php elseif($option['type'] == "setup"):?>
                                  (Setup Charge: <?=$option['price'];?> )
                              <?php else:?>
                                  ($<?=$option['price'];?>)
                              <?php endif;?>
                              
                              <?php if($option['price'] < 0){
                                  //we have a kit discount
                                  $kitDiscountAmount = $option['price'];
                              } else {
                                  $kitDiscountAmount = 0;
                              }
                              ?>
                          <?php endif;?>
                      <?php endif;?>
                    </div>
                  <?php endforeach;?>
                </div>
            <?php endif;?>
            <!-- End Options -->
          </td>
          <?php if($showPrices):?>
          <td class="cartRow" align="right" >
              <!--<input data-role="none" style="text-align:center;" type="text" size="1" name="quantity[<?=$cartid?>]" value="<?=intval($fields['quantity']);?>" id="Text1">-->
              <?php if (intval($fields['quantity']) > 29) :?> 
				  <input style="text-align:center;" type="text" size="3" name="quantity[<?=$cartid?>]" value="<?=intval($fields['quantity']);?>" id="cartQty">&nbsp; <a class="blue xlarge updateQty hide" href="#" data-toggle="tooltip" data-placement="top" title="update"><i class="fa fa-refresh blue large" aria-hidden="true"></i></a>
			  <?php else:?>
                  <select class="native-dropdown m-r-05" id="cartDD" name="quantity[<?=$cartid?>]">
                      <?php for ($i = 1; $i <= 29; $i++): ?>                  
                          <option <?php if (intval($fields['quantity']) == $i) { echo ('selected="" '); } ?> value="<?=$i;?>"><?=$i;?></option>
                      <?php endfor; ?>
                       <option <?php if (intval($fields['quantity']) == $i) { echo ('selected="" '); } ?> value="<?=$i;?>"><?=$i;?>+</option>
                      <!--<?php for ($i = 40; $i < 100; $i+=10): ?>                  
                          <option <?php if (intval($fields['quantity']) == $i) { echo ('selected="" '); } ?> value="<?=$i;?>"><?=$i;?></option>
                      <?php endfor; ?>
                      <?php for ($i = 100; $i <= 1000; $i+=100): ?>                  
                          <option <?php if (intval($fields['quantity']) == $i) { echo ('selected="" '); } ?> value="<?=$i;?>"><?=$i;?></option>
                      <?php endfor; ?>-->
                  </select>
              <?php endif;?>
                  <br><input name="delete[<?=$cartid?>]" data-role="none" type="submit" value="Remove Item" id="removeItem" class="xsmall">
                
          </td>
          <td class="cartRow" align="right" ><span class="red m-r-2"><?=$_Common->format_price($fields['line_total'],true);?></span>
              <?php if (intval($fields['quantity']) > 1):?>
                <div class="unit_price  xsmall m-r-2"><?=$_Common->format_price($fields['line_total'] / intval($fields['quantity']));?> each</div>
              <?php endif;?>
              <!--Check for Kit Discounts-->
              <?php if ($kitDiscountAmount < 0):?>
                <div class="unit_price cartRowDiscount m-r-2">You Saved $<?=$_Common->format_price(-$kitDiscountAmount * intval($fields['quantity']));?> </div>
              <?php endif;?>
          </td>
          <?php endif;?>
        </tr>
        <?php $kitDiscountAmount = 0; // set kit discount back to 0 for next item?>
      <?php endforeach;?>
      <!-- End of each cart row -->
      <?php if($showPrices):?>
      <tr>
        <td colspan="4"   class="cartRow">
            <div class="left m-t-05"> <strong>Total Weight:</strong>
                <?=$_Totals['totalWeight'];?>
              lbs.&nbsp;&nbsp;
            </div>
          <div class="cartFooter right lead m-b-0"><strong>Subtotal:</strong> (<?=$miniCart['item_count'];?> item<?php if($miniCart['item_count'] >1):?>s<?php endif;?>)<strong>
            <span class="red m-r-2"><?=$_Common->format_price($_Totals['subtotal'],true);?></span>
        </strong> </div></td>
      </tr>
      <?php endif;?>
		
                    <!-- Start UK Minimum -->
                    <!-- Check if cart subtotal is less than $200 and display notice-->				  
                    <?php if(($miniCart['total'] < 200)):?>
                      <tr>
                        <td colspan="4" class="cartRow">
                          <div class="cartFooter center red">
                            <big><strong>Notice: Brexit Tax Rules prevent us from shipping items to the UK and Northern Ireland if the Cart Subtotal is less than $200.00!</strong> </big>
                          </div>
                        </td>
                      </tr>
                    <?php endif;?>
                    <!-- End UK Minimum --> 
    </table>
    <div class="max-width-1000">
      <div class="inlineBlock pull-right m-r-2">
        
        <!-- CHeck for being on localhost or live site and set link accordingly -->
        <a href="<?php 
            echo (strpos($_SERVER['HTTP_HOST'], 'localhost') !== false) 
                ? $_SERVER['HTTP_HOST'] . '/../checkout.php?cart_id=' . session_id() 
                : $secure_url . '/checkout.php?cart_id=' . session_id(); 
        ?>">
          <button class="btn btn-danger m-l-05" type="button">
            Checkout
            <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span>
          </button>
        </a>
      </div>
    </div>
  </form>
  <?php if($googleButton):?>
  <p> <br />
    <b>- or you can use -</b><br />
    <?=$googleButton;?>
  </p>
  <?php endif;?>
  <?php $relatedItemsCount = count($relatedItems);?>
  <?php $relatedItemsCount = 0;?><!--Marcello turned this off for now -->
  <?php if($relatedItemsCount > 0):?>
  <br clear="all">
  <br clear="all">
  <h4 align="left" style="line-height:10px;">Customers who bought the &quot;
    <?=$lastItem;?>
    &quot; also bought</h4>
  <div style="padding-left:25px;">
    <table border="0" cellpadding="3" cellspacing="0" width="100%" ID="Table1">
      <?php foreach($relatedItems as $i=>$fields):?>
      <?php if($i == 5){break;}?>
      <?php $thumbnail = trim($fields['thumbnail']);?>
      <tr>
        <td valign="top"><?php if($thumbnail != "" && file_exists("images/thumbs/$thumbnail")):?>
          <img src="images/thumbs/<?=$thumbnail;?>" border="0">
          <?php else:?>
          <li>
            <?php endif;?>
        </td>
        <td valign="top" width="98%" ><?php if(substr($fields['page'],-4) == "html" || substr($fields['page'],-3) == "htm"):?>
          <a href="../<?=$fields['page'];?>">
          <?=$fields['name'];?>
          </a>
          <?php else:?>
          <a href="products.php?sku=<?=$fields['sku'];?>&detail=yes">
          <?=$fields['name'];?>
          </a>
          <?php endif;?>
        </td>
      </tr>
      <?php endforeach;?>
    </table>
  </div>
  <?php endif;?>
</div>
<?php endif;?>
</body>
</html>
