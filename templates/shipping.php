<?php
$debug = false;
$shippingPlugins = array();
// check the shipping table to see if Shipping Plugin Name is added and
// "Use Shipping Plugin" is on. If so, try to dynamically load it here
$shipExt = $_DB->getRecord("SELECT use_shipping_plugin,shipping_plugin_name FROM shipping");
if($shipExt['use_shipping_plugin'] == 'true' && trim($shipExt['shipping_plugin_name']) != ""){
	$fileNames = explode(',',trim($shipExt['shipping_plugin_name']));
	foreach($fileNames as $i=>$name){
		$flds = explode('.',$name);
		$shippingPlugins[] = $flds[0];	
	}
}
$dontShip = false;
$_SESSION['no_subscription'] = "No";
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
	<head> 
<title><?=$pageTitle;?></title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">
<link rel="stylesheet" type="text/css" href="../../_css/bootstrap.css" />
<link type="text/css" rel="stylesheet" href="../../_css/responsive.css">
<link type="text/css" rel="stylesheet" href="../../_css/nav.css">
<link type="text/css" rel="stylesheet" href="../../_css/general.css">
<link type="text/css" rel="stylesheet" href="../../_css/product.css">
<link rel="stylesheet" type="text/css" href="../../_css/cart.css" />
	    
</head>
<body onLoad="submitShipMethodChange(this.form,'shipping_method');">

<!--webbot bot="PurpleText" PREVIEW="
This page contains PHP script variables in the HTML that may be hidden in your editor.
So, please be careful editing this page and be sure to keep a backup copy before overwriting it.
View the HTML source code for more details.
"-->
	
	<?php if(count($_CART) == 0): ?>    
        <div align="center">
          <h2>Your Cart is empty</h2>    
            <div class="inlineBlock m-l-3">
              <a href="<?=$_CF['basics']['home_page_name'];?>">
                <div class="largeSubmitButtonImg inlineBlock arrowLeft left">&nbsp;</div>
                <div class="largeSubmitLink inlineBlock">Continue Shopping</div>
                </a>
            </div>
        </div>
	<?php else:?>

		<script type="text/javascript" src="javascripts/checkout.js"></script>
		<script type="text/javascript">
			var orderSubmitted = "<?=$_SESSION['orderSubmitted'];?>";
			function clickOnce(){
				if(orderSubmitted != "0"){
					alert("Your order has already been processed.\n\n" +
						"If you feel there is a problem with the order, " +
						"please contact <?=$_CF['email']['store_email_address'];?> to report it.");
					return false;
				}
			return true;
			}
		</script>
		<!--<style>
			td{ vertical-align: middle; }
			.topAlign{ align: left; vertical-align: top; }
		</style>-->

		<?php
			error_reporting(E_PARSE|E_WARNING);
			$showPrices = $_CF['cart']['show_prices'];
			$colspan = 4;
			if(!$showPrices){
				$colspan = 2;
			}
			$haveShippingRates = false;
			//if(count($shippingRates) > 0){
      if (isset($shippingRates) && is_array($shippingRates) && count($shippingRates) > 0) {
				$haveShippingRates = true;
			}
			$showDomestic = true;
			$taxFldNames = array('GST','HST','PST','VAT');
			foreach($taxFldNames as $k=>$fldName){
				if(isset($_Totals[$fldName]) && $_Totals[$fldName] > 0){
					$showDomestic = false;
					break;
				}
			}
		?>

		<link rel="stylesheet" type="text/css" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.1.0/css/font-awesome.min.css" /><!--marcello-->
        <div class="row m-x-0 text-left max-width-1000">
          <h2 class="text-center col-xs-12"><?=$pageTitle;?></h2>
          <form name="process" method="post" action="process.php" onSubmit="return checkRequiredFields(this);" ID="Form2" data-toggle="validator" role="form">
            <div class="addressContainer">
              <!--Bill to-->
              <div id="billToContainer" class="m-b-2 col-sm-offset-1 col-sm-4 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12">
                <div class="billToMsg">
                  <?php if($showPrices):?>
                    <h4 id="message" class="red well well-xs m-b-0">Bill To</h4>
                  <?php else:?>
                    <h4 id="message" class="red well well-xs m-b-0">Contact Information</h4>
                  <?php endif;?>
                </div>
                <div class="billToInfo m-l-3">              
                    <?php if(!empty($_SESSION['billaddress_companyname'])):?>
                    <?=$_SESSION['billaddress_companyname'];?><br />
                    <?php endif;?>
                    
                    <?=$_SESSION['billaddress_firstname'];?> <?=$_SESSION['billaddress_lastname'];?><br />
                    <?=$_SESSION['billaddress_addr1'];?><br />
                    
                    <?php if(!empty($_SESSION['billaddress_addr2'])):?>
                    <?=$_SESSION['billaddress_addr2'];?><br />
                    <?php endif;?>
                    
                    <?=$_SESSION['billaddress_city'];?>,
                    <?=$_SESSION['billaddress_state'];?>,
                    <?=$_SESSION['billaddress_country'];?>
                    <?=$_SESSION['billaddress_postalcode'];?>
                    <br />
                    Phone: (<?=$_SESSION['billaddress_areacode'];?>)
                    <?=$_SESSION['billaddress_phone'];?>
                    <!--&nbsp;&nbsp;Evening: (<?=$_SESSION['billaddress_areacode2'];?>)
                  <?=$_SESSION['billaddress_phone2'];?>-->
                    <br />
                    <?=$_SESSION['billaddress_email'];?>
                </div>
              </div>
              <!--divider left-->
              <!--<div id="centerDividerLeft" class="centerDivider col-sm-offset-0 col-sm-1 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12"></div>-->
              <!--Ship to-->
              <div class="p-x-0-xs col-sm-offset-1 col-sm-5 borderLeft-sm  ">
                <div id="shipToContainer" class="p-r-0 p-r-2-xs col-sm-offset-2 col-sm-10  col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12">
                  <div class="shipToMsg">
                    <?php if($_CF['shipping']['require_shipping']):?>
                    <h4 id="" class="red well well-xs m-b-0">Ship To</h4>
                    <?php else:?>
                      &nbsp;
                    <?php endif;?>
                  </div>
                  <div class="shipToInfo m-l-3">              
                      
                      <?php if($_CF['shipping']['require_shipping']):?>
                      <?php if(!empty($_SESSION['shipaddress_companyname'])):?>
                      <?=$_SESSION['shipaddress_companyname'];?><br />
                      <?php endif;?>
                      
                      <?=$_SESSION['shipaddress_firstname'];?> <?=$_SESSION['shipaddress_lastname'];?><br />
                      <?=$_SESSION['shipaddress_addr1'];?><br />
                      
                      <?php if(!empty($_SESSION['shipaddress_addr2'])):?>
                      <?=$_SESSION['shipaddress_addr2'];?><br />
                      <?php endif;?>
                      
                      <?=$_SESSION['shipaddress_city'];?>,
                      <?=$_SESSION['shipaddress_state'];?>,
                      <?=$_SESSION['shipaddress_country'];?>
                      <?=$_SESSION['shipaddress_postalcode'];?>
                      <br />
                    Phone: (<?=$_SESSION['shipaddress_areacode'];?>)
      <?=$_SESSION['shipaddress_phone'];?>
      <br />
                      <?=$_SESSION['shipaddress_email'];?>
                      <?php else:?>
                      &nbsp;
                      <?php endif;?>
                  </div>
                </div>
              </div>
            </div>
            
            <div class="col-sm-12 p-x-2">
              <table class="table table-striped table-condensed table-responsive  m-b-1" ID="Table2">
                <tr>
                  <?php if($showPrices):?>
                    <th class="cartHeader center"  colspan="2">Your Order</th>
                    <th class="cartHeader text-right" ><span class="m-r-1">Qty</span></th>
                  <th class="cartHeader text-right"  ><span class=" m-r-2">Total</span></th>
                  <?php else:?>
                    <th class="cartHeader center"  >&nbsp;</th>
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
                        <span class="alert-sm alert-white m-r-1"><?=intval($fields['quantity']);?></span>
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
                    <!-- Start Subtotal -->
                    <tr>
                      <td colspan="3" class="cartRow">
                        <div class="cartFooter right ">
                          <big><strong>Subtotal:</strong> (<?=$miniCart['item_count'];?> item<?php if($miniCart['item_count'] >1):?>s<?php endif;?>)</big>
                        </div>
                      </td>
                      <td class="cartRow">
                          <big><strong class="red right m-r-2"><?=$_Common->format_price($_Totals['subtotal'],true);?></strong></big>
                      </td>
                    </tr>
                    <!-- End Subtotal -->
                    <!-- Start Discount -->
                    <?php if(isset($_Totals['discount']) && $_Totals['discount'] > 0):?>
                        <tr class="cartRowDiscount">
                          <td colspan="3" class="cartRow">
                            <div class="cartDiscount right">
                              <strong class="">
                                <?php if(!empty($_SESSION['discount_text'])):?>
                                    <?=$_SESSION['discount_text'];?>:
                                <?php else:?>
                                    Discount:
                                <?php endif;?>
                              </strong>
                            </div>
                          </td>
                          <td>
                              <span class="alert-success right m-r-2">- <?=$_Common->format_price($_Totals['discount'],true);?></span>
                          </td>
                        </tr>
                    <?php endif;?>
                    <!-- End Discount -->
                    
                    <!-- Start UK Minimum -->
                    <!-- Check if country is UK and subtotal is less than $200 -->	
				    <?php $dontShip = (($_SESSION['shipaddress_country'] == "GB") || ($_SESSION['shipaddress_country'] == "EN") || ($_SESSION['shipaddress_country'] == "NB")) && ($miniCart['total'] < 200);?>
                    <?php if ($dontShip):?>
                      <tr>
                        <td colspan="4" class="cartRow">
                          <div class="cartFooter center onSaleInv marginT10px">
                            <big><strong>Notice: Brexit Tax Rules prevent us from shipping items to the UK and Northern Ireland if the Subtotal is less than $200.00!</strong> </big>
                          </div>
                          <div class="cartFooter center red marginT10px">
                            <big><br><strong>Please <button class="btn btn-lightRed cursorDefault" type="button"><a href="<?=$_CF['basics']['home_page_name'];?>"><strong>Continue Shopping</strong></a></button> 
                            until you your Cart Subtotal is more than $200.00!</strong> <br><br></big>
                          </div>
                        </td>
                      </tr>
                    <?php endif;?>
                    <!-- End UK Minimum -->
                    <!-- Start Shipping -->
				    <?php if ($debug) {
                        echo("<p>  haveShippingRate: " . $haveShippingRates . "</p> <p>&nbsp;</p>");
                        $_Common->debugPrint($_SESSION['shipaddress_firstname'],"firstname: ");
                      }
                    ?>
                    <?php if($haveShippingRates):?>
				   <?php if ($debug) {echo("<p>  got here: " . $haveShippingRates . "</p> <p>&nbsp;</p>");}?>
                      <tr>
                        <td colspan="3" class="cartRow" align="right">
                          <div class="form-inline">
                            <!--<span  class="alert-sm alert-danger inlineBlock "><strong>Ship&nbsp;Method:</strong></span>--> 
                           <button class="btn btn-lightRed cursorDefault" type="button">
                            <strong>Ship&nbsp;Method:</strong></button>
                            <select class="form-control" name="preferred_shipper" onChange="submitShipCarrierChange(this.form,'preferred_shipper');">
                              <?php foreach($shippingPlugins as $k=>$shipper):?>
                                  <?php if($_SESSION['preferred_shipper'] == $shipper):?>
                                  <option value="<?=$shipper;?>" selected><?=strtoupper($shipper);?></option>
                                  <?php else:?>
                                  <option value="<?=$shipper;?>"><?=strtoupper($shipper);?></option>
                                  <?php endif;?>
                              <?php endforeach;?>
                              </select>							
                            <select class="form-control m-y-1" name="shipping_method" onChange="submitShipMethodChange(this.form,'shipping_method');">
                              <?php foreach($shippingRates as $carrier=>$price):?>
                                  <?php if($selectedCarrier != "" && $carrier == $selectedCarrier):?>
                                      <option value="<?=$carrier;?>" selected><?=$carrier;?> - <?=$price;?></option>
                                  <?php else:?>
                                      <option value="<?=$carrier;?>"> <?=$carrier;?> - <?=$price;?></option>
                                  <?php endif;?>
                              <?php endforeach;?>
                              </select>
                              <div class="alert-danger text-center"> Use USPS for small domestic, international & APO orders! </div>
                          </div>
                          </td>
                        <td id="shippingAmount" align="right" class="cartRow"><span class="red m-r-2"><?=$_Common->format_price($_Totals['shipping'],true);?></span></td>
                      </tr>
                    <?php elseif($_Totals['shipping'] > 0):?>
                      <tr>
                        <td colspan="3" align="right" class="cartRow">Shipping:</td>
                        <td id="shippingAmount" align="right" class="cartRow"><span class="red m-r-2"><?=$_Common->format_price($_Totals['shipping'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    <!-- End Shipping -->
                    
                    <!-- Start Tax etc. -->
                    <?php $colspan=3; ?>
                    <?php if(isset($_Totals['insurance']) && $_Totals['insurance'] > 0):?>
                      <tr>
                        <td colspan="<?=$colspan;?>" align="right">UPS Heavy Item Surcharge:</td>
                        <td align="right" ><span class="red m-r-2"><?=$_Common->format_price($_Totals['insurance'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    
                    <?php if(isset($_Totals['GST']) && $_Totals['GST'] > 0):?>
                      <tr>
                        <td colspan="<?=$colspan;?>" align="right">GST:</td>
                      <td id="gstAmount" align="right" ><span class="red m-r-2"><?=$_Common->format_price($_Totals['GST'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    
                    <?php if(isset($_Totals['HST']) && $_Totals['HST'] > 0):?>
                      <tr>
                        <td colspan="<?=$colspan;?>" align="right">HST:</td>
                      <td id="hstAmount" align="right" ><span class="red m-r-2"><?=$_Common->format_price($_Totals['HST'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    
                    <?php if(isset($_Totals['PST']) && $_Totals['PST'] > 0):?>
                      <tr>
                        <td colspan="<?=$colspan;?>" align="right">PST:</td>
                      <td id="pstAmount" align="right" ><span class="red m-r-2"><?=$_Common->format_price($_Totals['PST'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    
                    <?php if(isset($_Totals['VAT']) && $_Totals['VAT'] > 0):?>
                      <tr>
                        <td colspan="<?=$colspan;?>" align="right">VAT:</td>
                      <td id="vatAmount" align="right" ><span class="red m-r-2"><?=$_Common->format_price($_Totals['VAT'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    
                    <?php if($showDomestic && $_Totals['salestax'] > 0):?>
                      <tr>
                        <td colspan="<?=$colspan;?>" align="right"><?=$_SESSION['shipaddress_state'];?> Sales Tax:</td>
                      <td id="salestaxAmount" align="right" ><span class="red m-r-2"><?=$_Common->format_price($_Totals['salestax'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    <!-- End Tax etc. -->
                    
                    <!-- Start Order Total -->
                    <tr>
                      <td colspan="4" class="p-y-0" align="right"><span class=" m-r-2">------</span></td>
                    </tr>
                    <tr>
                      <td colspan="<?=$colspan;?>" align="right">
                          <span class="left m-t-1 small">Total Weight: 
                          <?=$_Totals['totalWeight'];?>
                          lbs.</span>
                          <span class="lead m-b-0"><big><strong>Order Total:</strong></big></span></td>
                      <td id="grandtotalAmount" align="right" class="lead m-b-0 red"><big><strong><?=$_Common->format_price($_Totals['grandtotal'],true);?></strong></big></td>
                    </tr>
                    <!-- End Order Total -->
                <?php endif;?>
              </table>
            </div>
            
			
			<?php  if (!($dontShip)):?>   
			  
                <?php if($paymentPage != ""):?>
                  <div class="row clearBoth p-x-2 m-x-0">
                    <?=$paymentPage;?>
                  </div>
                <?php endif;?>

                <div class="row clearBoth p-x-2 m-x-0">
                  <div class="p-x-2 alert alert-mmd col-xs-12 m-t-1">
                    <h2 class="p-x-2 m-x-0 m-t-0">Comments and Preferences</h2>
                    <div class="col-xs-6 col-xxs-12">

                      <div class="form-group has-feedback">
                        <label for="orderComments">Comments:</label>
                        <textarea class="form-control" name="order_comments" placeholder="Your comments" cols="30" rows="3" id="orderComments"> </textarea>
                        <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                        <div class="help-block with-errors"></div>
                      </div>
                      <div class="form-group">
                        <div class="form-inline row">
                          <label class="control-label col-xs-5">DiscSox newsletter<span class="blue">*</span>:</label>
                          <div class="col-xs-7">
                              <label class="checkbox-inline">
                                  <input name="no_subscription" type="checkbox" value="Yes" unchecked>
        Get info about new products and promotions
                              </label>
                          </div>
                        </div> 
                      </div>
                      <span class="blue alert-info center">*MMDesign does not send Spam or sell e-mail addresses to anybody! </span>
                    </div>
                    <div class="col-xs-6 col-xxs-12">
                      <div class="form-group has-feedback">
                        <label for="seEngineRef">How did you hear about us?</label>
                        <select class="form-control" name="se_engine_ref" id="seEngineRef">
          <option value="Please select" selected>Please Select</option>
                                          <option value="Amazon">Amazon</option>
                                          <option value="The Container Store">The Container Store</option>
                                          <option value="CyberGuys">CyberGuys</option>
                                          <option value="J&R">J&amp;R</option>
                                          <option value="My Home My Style">My Home My Style</option>
                                          <option value="Professional Organizer">Professional Organizer</option>
                                          <option value="Real Simple">Real Simple</option>
                                          <option value="Smarthome">Smarthome</option>
                                          <option value="X-TREME Geek">X-TREME Geek</option>
                                          <option value="Web Search">Web Search</option>
                                          <option value="Facebook">Facebook</option>
                                          <option value="Pinterest">Pinterest</option>
                                          <option value="Twitter">Twitter</option>
                                          <option value="YouTube">YouTube</option>
                                          <option value="Other">Other</option>
                        </select>
                        <span class="glyphicon form-control-feedback m-r-2" aria-hidden="true"></span>
                        <div class="help-block with-errors"></div>
                      </div>

                      <div class="form-group has-feedback">
                        <label for="se_key_words">Web Search:</label>
                        <input class="form-control" name="se_key_words" type="text" id="se_key_words" value="" placeholder="I searched for...">
                        <span aria-hidden="true" class="glyphicon form-control-feedback"></span>
                        <div class="help-block with-errors"></div>
                      </div>

                      <div class="form-group has-feedback">
                        <label for="other_ref">Other:</label>
                        <input class="form-control" name="other_ref" type="text" id="other_ref" value="" placeholder="Please specify..." >
                        <span aria-hidden="true" class="glyphicon form-control-feedback"></span>
                        <div class="help-block with-errors"></div>
                      </div>
                    </div>
                  </div>
                </div>

                <?php
                    $buttonText = "Place Secure Order";
                    if(!$showPrices){
                        $buttonText = "Submit Quote";
                    }						

                    function getRemoteIP ()
                    {  
                      // check to see whether the user is behind a proxy - if so,
                      // we need to use the HTTP_X_FORWARDED_FOR address (assuming it's available)
                      if (strlen($_SERVER["HTTP_X_FORWARDED_FOR"]) > 0) { 
                        // this address has been provided, so we should probably use it
                        $f = $_SERVER["HTTP_X_FORWARDED_FOR"];
                        // however, before we're sure, we should check whether it is within a range 
                        // reserved for internal use (see http://tools.ietf.org/html/rfc1918)- if so 
                        // it's useless to us and we might as well use the address from REMOTE_ADDR
                        $reserved = false;
                        // check reserved range 10.0.0.0 - 10.255.255.255
                        if (substr($f, 0, 3) == "10.") {
                          $reserved = true;
                        }
                        // check reserved range 172.16.0.0 - 172.31.255.255
                        if (substr($f, 0, 4) == "172." && substr($f, 4, 2) > 15 && substr($f, 4, 2) < 32) {
                          $reserved = true;
                        }
                        // check reserved range 192.168.0.0 - 192.168.255.255
                        if (substr($f, 0, 8) == "192.168.") {
                          $reserved = true;
                        }
                        // now we know whether this address is any use or not
                        if (!$reserved) {
                          $ip = $f;
                        }
                      } 
                      // if we didn't successfully get an IP address from the above, we'll have to use
                      // the one supplied in REMOTE_ADDR
                      if (!isset($ip)) {
                        $ip = $_SERVER["REMOTE_ADDR"];
                      }
                      // done!
                      return $ip;
                    }
                    $_SESSION['ip_address'] = getRemoteIP();	
                    //echo $_SESSION['ip_address'];

                    //Fraud Detection
                    $fraudulant = false;
                    //Get fraud emails from database
                    $fraud_emails = $_CF['fraud_detection']['fraud_emails'];
                    //Extract indiviual emails
                    $fraud_email_values = explode(" ",$fraud_emails);
                    //Check each email against current email in cart
                    foreach($fraud_email_values as $fraud_email){
                        if($_SESSION['billaddress_email'] == $fraud_email){  
                          $fraudulant = true;
                        }	
                    }
                 ?> 
                <div class="text-center col-xs-12">
                  <div class="form-group">  
                    <button type="submit" class="btn btn-danger submit-order-button" name="submit_order" value="<?=$buttonText;?>" ID="submit_order2"  data-placement="top" data-toggle="tooltip" data-html="true" data-original-title='
                    <?php if($payment_method == "credit_card.html"):?>Please do not hit the button twice! <br>                    
                    Credit card verification can take up to 2 minutes!
                    <?php elseif($payment_method == 'mail_in_payment.html'):?>Click to place check order!
                    <?php elseif($payment_method == "purchase_order.html"):?>Click to place P.O.!
                    <?php else:?> Click to place PayPal order!
                    <?php endif;?> ' data-loading-text="<i class='fa fa-spinner fa-spin '></i> Processing Order" >Place Secure Order <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
                    <div class="col-xxs-12 col-xs-7 col-md-8 ">

                      <div class="form-group has-feedback">
                        <div class="form-inline row">
                          <label class="control-label col-xs-5">GDPR Agreement<span class="red">*</span>:</label>
                          <div class="col-xs-7">
                              <label class="checkbox-inline textLeft">
                                  <input class="form-control" name="gdpr_agree" type="checkbox" value="Yes" data-error="GDPR Agreement is required!" required >
        I consent to having MMDesign store my submitted information so my order can be processed.
                              Review our <a href="../../privacy.php" target="_blank">Privacy Policy</a>.</label>
                          <div class="help-block with-errors"></div>
                          </div>
                        </div> 
                      </div>
                    </div>
                  </div>
                  <div class="col-xxs-12 clearBoth">
                    All your personal information, including phone number, name, and address is encrypted so that it cannot be read as the information travels over the Internet. No credit card information is stored on our servers!<br>
                    <strong>Every transaction you make at MMDesign.com is 100% safe.</strong>
                    <br><div class="textLeft"><span class="red">*</span> Required entries</div>
                  </div>  
                </div>
			  
			<?php endif;?>
          
          </form>
		</div>


		<!-- this is to allow us to change the shipping method and reload the page. DO NOT REMOVE -->
		<?php if($haveShippingRates):?>
		<form name="shipform" action="shipping.php" method="get" ID="Form1">
			<input type="hidden" name="shipping_method" value="" ID="Hidden1">
			<input type="hidden" name="continue" value="true" ID="Hidden2">
			<input type="hidden" name="payment_method" value="<?=$_REQUEST['payment_method'];?>" ID="Hidden3">
			<input type="hidden" name="coupon" value="<?=$_REQUEST['coupon'];?>" ID="Hidden4">
		</form>
		<?php endif;?>

		<?php if($haveShippingRates):?>
		<form name="shipform2" action="shipping.php" method="get" ID="Form3">
			<input type="hidden" name="continue" value="true" ID="Hidden5">
			<input type="hidden" name="payment_method" value="<?=$_REQUEST['payment_method'];?>" ID="Hidden6">
			<input type="hidden" name="coupon" value="<?=$_REQUEST['coupon'];?>" ID="Hidden7">
			<input type="hidden" name="preferred_shipper" value="" ID="Hidden8">
		</form>
		<?php endif;?>

	<?php endif;?>
  <!--<p>Print _Totals</p></p>
<p>&nbsp;<?=print_r($_Totals); ?></p>	-->
<!--<p>County Variable Value:&nbsp;<?=print($_SESSION['shipaddress_county']); ?></p>-->

	<script type="text/javascript">
		var createAccountUsername = "<?=$_SESSION['registration']['username'];?>";
		var createAccountPassword = "<?=$_SESSION['registration']['password'];?>";
		var checkIsRegistered = "<?=$_SESSION['isRegistered'];?>";


		if(checkIsRegistered == ""){
			checkAccountInfo();
		}
		function checkAccountInfo(){
			if((createAccountUsername == "") || (createAccountPassword == "")){
				alert("You did not specify a Login Email and/or Password\n\n" +
					"No Account will be created and you will not be able to review your orders online.\n" +
					"If this is correct, just continue.\n\n" +
					"Otherwise please hit the browser back button and enter a Login Email and Password.");
				return false;
			}
			if(!emailCheck(createAccountUsername)){
				alert("You did not enter a valid Login Email Address\n" +
					  "Please hit the browser back button and enter correct Login Email and Password!");
				return false;
			}
		return true;
		}

		function emailCheck(str){
			var at="@"
			var dot="."
			var lat=str.indexOf(at)
			var lstr=str.length
			var ldot=str.indexOf(dot)
			if (str.indexOf(at)==-1){
				return false;
			}
			if (str.indexOf(at)==-1 || str.indexOf(at)==0 || str.indexOf(at)==lstr){
				return false;
			}
			if (str.indexOf(dot)==-1 || str.indexOf(dot)==0 || str.indexOf(dot)==lstr){
				return false;
			}
			if (str.indexOf(at,(lat+1))!=-1){
				return false;
			}
			if (str.substring(lat-1,lat)==dot || str.substring(lat+1,lat+2)==dot){
				return false;
			}
			if (str.indexOf(dot,(lat+2))==-1){
				return false;
			}
			if (str.indexOf(" ")!=-1){
				return false;
			}
			return true;
		}
</script>
</body>
</html>