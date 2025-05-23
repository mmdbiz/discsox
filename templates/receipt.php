<?php
// Initialize missingFields as an empty array if not set
$missingFields = $missingFields ?? []; // PHP 7.4+ syntax
// or
if (!isset($missingFields)) {
    $missingFields = [];
}
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
<body>
<!--The following code syncronizes the customer e-mail preferences with Constant Contact-->

<!--End Constant Contact Code-->
<!--webbot bot="PurpleText" PREVIEW="
This page contains PHP script variables in the HTML that may be hidden in your editor.
So, please be careful editing this page and be sure to keep a backup copy before overwriting it.
View the HTML source code for more details.
"-->

  <div align="center" class="">

	<?php if(count($_CART) == 0): ?>
		
        <div>
          <h2>Your Cart is empty</h2>    
            <div class="inlineBlock m-l-3">
              <a href="<?=$_CF['basics']['home_page_name'];?>">
                <div class="largeSubmitButtonImg inlineBlock arrowLeft left">&nbsp;</div>
                <div class="largeSubmitLink inlineBlock">Continue Shopping</div>
                </a>
            </div>
        </div>

		<?php elseif(count($missingFields) > 0): ?>

  		<form name="process" method="post" action="process.php" ID="Form1">
  			<table border="0" width="100%" cellspacing="1" ID="Table1">
  				<tr>
  					<td colspan="2" valign="top" align="center"><h4>The following fields are required to process your order:</h4></td>
  				</tr>
  				<?php foreach($missingFields as $fldName=>$fldText):?>
  				<?php
  					$size = 40;
  					$max = 40;
  					if($fldName == "card_number"){
  						$size = 16;
  						$max = 16;
  					}
  					elseif($fldName == "expire_month"){
  						$size = 2;
  						$max = 2;
  					}
  					elseif($fldName == "expire_year" || $fldName == "CVV2"){
  						$size = 4;
  						$max = 4;
  					}
  				?>
  				<tr>
  					<td align="right"><?=$fldText;?></td>
  					<td>
  						<input type="text" name="<?=$fldName;?>" value="" size="<?=$size;?>" maxlength="<?=$max;?>" ID="Text1">
  					</td>
  				</tr>
  				<tr>
  					<td colspan="2">&nbsp;</td>
  				</tr>
  				<tr>
  					<td colspan="2" valign="top" align="center">
  						<input type="submit" name="submit" value="Submit Order" ID="Submit1">
  					</td>
  				</tr>
  				<?php endforeach;?>
  			</table>
  		</form>

	<?php elseif($error):?>

		<table border="0" width="100%" cellspacing="1" ID="Table2">
		<tr>
			<td width="100%" valign="top" align="center">
				<h4><br>Program Error</h4>
				<font color=blue><?=$error;?></font><br><br>
			</td>
		</tr>
		</table>

	<?php else:?>

		<?php
			$pickup = false;
			if(isset($_CF['shipping']['offer_local_pickup']) && $_CF['shipping']['offer_local_pickup']){
				if($selectedCarrier == $_CF['shipping']['local_pickup_text']){
					$pickup = true;
				}
			}
			error_reporting(E_PARSE|E_WARNING);
			$showPrices = $_CF['cart']['show_prices'];
			$orderLabel = "Order";
			$colspan = 4;
			if(!$showPrices){
				$orderLabel = "Quote";
				$colspan = 2;
			}
		?>
		
        <div class="row m-x-0 text-left max-width-1000">
          <h2 class="text-center col-xs-12"><?=$pageTitle;?></h2>
          <div class="col-sm-12 ">
              <div class="col-sm-6 backgroundLightGray"><?=$orderLabel;?> Date: <?=$order_date;?></div>
              <div class="col-sm-6 backgroundLightGray "><?=$orderLabel;?> Number: <?=$order_number;?></div>
          </div>
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
            <div class="p-x-0-xs col-sm-offset-1 col-sm-5 borderLeft-sm  ">
            <!--Ship to-->
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
                    
                    <!-- Start Shipping -->
					<?php if($pickup || (isset($_Totals['shipping']) && $_Totals['shipping'] > 0)):?>
                      <tr>
                        <td colspan="3" class="cartRow" align="right"><?=$selectedCarrier;?>:</td>
                        <td class="cartRow" align="right" ><span class="red m-r-2"><?=$_Totals['shipping'];?></span></td>
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
                    
				            <?php $showDomestic = true; ?>
                    <?php if(isset($_Totals['GST']) && $_Totals['GST'] > 0):?>
				              <?php $showDomestic = false; ?>
                      <tr>
                        <td colspan="<?=$colspan;?>" align="right">GST:</td>
                      <td id="gstAmount" align="right" ><span class="red m-r-2"><?=$_Common->format_price($_Totals['GST'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    
                    <?php if(isset($_Totals['HST']) && $_Totals['HST'] > 0):?>
				              <?php $showDomestic = false; ?>
                      <tr>
                        <td colspan="<?=$colspan;?>" align="right">HST:</td>
                      <td id="hstAmount" align="right" ><span class="red m-r-2"><?=$_Common->format_price($_Totals['HST'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    
                    <?php if(isset($_Totals['PST']) && $_Totals['PST'] > 0):?>
				              <?php $showDomestic = false; ?>
                      <tr>
                        <td colspan="<?=$colspan;?>" align="right">PST:</td>
                      <td id="pstAmount" align="right" ><span class="red m-r-2"><?=$_Common->format_price($_Totals['PST'],true);?></span></td>
                      </tr>
                    <?php endif;?>
                    
                    <?php if(isset($_Totals['VAT']) && $_Totals['VAT'] > 0):?>
				              <?php $showDomestic = false; ?>
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
        </div>

		<div align="left">

		    <!--<br clear="all" />
		    
			<?php if(!empty($downloadLinks) && count($downloadLinks) > 0):?>
				<p>&nbsp;</p>
				<table border="0" cellspacing="0" cellpadding="3" width="100%" ID="Table5">
				<tr>
					<th align="left">Your Downloads</th>
				</tr>
				<tr>
					<td style="padding-top:10px;">
						Click on the links below to download your order.
					</td>
				</tr>
				<?php foreach($downloadLinks as $j=>$link):?>
				<tr>
					<td><li><?=$link;?></td>
				</tr>
				<?php endforeach;?>
				</table>
			<?php endif;?>-->
		</div>
		<div class="center"><a href="#" onClick="javascript:window.print()" title="print page"><span class="glyphicon glyphicon-print lead" aria-hidden="true"></span></a> Print this page for your records. </div>
		
        <!--Add Tracking Code--><!-- #BeginLibraryItem "/Library/-CodeAnalytics.lbi" -->
<!--Google Analytics Code -->
<script type="text/javascript">
var gaJsHost = (("https:" == document.location.protocol) ? "https://ssl." : "http://www.");
document.write(unescape("%3Cscript src='" + gaJsHost + "google-analytics.com/ga.js' type='text/javascript'%3E%3C/script%3E"));
</script>
<script type="text/javascript">
try{ 
var pageTracker = _gat._getTracker("UA-1342362-1");
pageTracker._trackPageview();
} catch(err) {} 
</script><!-- #EndLibraryItem --><!-- Start the order tracking here-->
        <script type="text/javascript">
			var pageTracker = _gat._getTracker("UA-1342362-1");
			pageTracker._trackPageview();
			
			<?php if($order_number > 0):?>
			
			  pageTracker._addTrans(
				"<?=$order_number;?>", // Order ID
				"", // Affiliation
				"<?=$_Totals['grandtotal'];?>", // Total
				"<?=$_Totals['salestax'];?>", // Total
				"<?=$_Totals['shipping'];?>", // Shipping
				"<?=$_SESSION['billaddress_city'];?>", // City
				"<?=$_SESSION['billaddress_state'];?>", // State
				"<?=$_SESSION['billaddress_country'];?>" // Country
			  );
			
			  <?php foreach($_CART as $i=>$row):?>
				pageTracker._addItem(
				  "<?=$order_number;?>", // Order ID
				  "<?=$row['sku'];?>", // SKU
				  "<?=$row['name'];?>", // Product Name
				  "", // Category
				  "<?=$row['price'];?>", // Price
				  "<?=$row['quantity'];?>" // Quantity
				);
			  <?php endforeach;?>
			
			  pageTracker._trackTrans();
			
			<?php endif;?>

        </script> 
	<?php endif;?>    
    <!-- ////////////////// Start Google Code for Discsox Adwords Conversion Page //////////////////-->
		<script type="text/javascript">
		  /* <![CDATA[ */
		  var google_conversion_id = 1010098515;
		  var google_conversion_language = "en";
		  var google_conversion_format = "3";
		  var google_conversion_color = "ffffff";
		  var google_conversion_label = "dB7wCM3_vgIQ08LT4QM";
		  var google_conversion_value = 0;
		  /* ]]> */
        </script>
        <script type="text/javascript" src="https://www.googleadservices.com/pagead/conversion.js">
        </script>
        <noscript>
          <div style="display:inline;">
            <img height="1" width="1" style="border-style:none;" alt="" src="https://www.googleadservices.com/pagead/conversion/1010098515/?label=dB7wCM3_vgIQ08LT4QM&amp;guid=ON&amp;script=0"/>
          </div>
        </noscript>

    <!--////////////////// Start REGISTER EMAIL WITH CONSTANT CONTACT ///////////////////////-->
    <?php
//init some parameters
$debug = false; //shows responses and requests
$debug2 = false; //when true info is not coming from the shopping cart 
if ($debug2) {
	$subscription = "Yes";
	//$email = urlencode(strtolower("ben@discsox.com"));
	$email = (strtolower("benjamin@discsox.com"));
	$first_name = "Benjamin";
	$last_name = "Pascua";
	$state = "California";
	$country = "USA";
	$zipcode = "95030";
} 
else {
	$subscription = $_SESSION['no_subscription'];
	$email = (strtolower($_SESSION['billaddress_email']));
	$first_name = $_SESSION['billaddress_firstname'];
	$last_name = $_SESSION['billaddress_lastname'];
	$zipcode = $_SESSION['billaddress_postalcode'];
	$addr1 = $_SESSION['billaddress_addr1'];
	$addr2 = $_SESSION['billaddress_addr2'];
	$city = $_SESSION['billaddress_city'];
	$state = $_SESSION['billaddress_state'];
	$country = $_SESSION['billaddress_country'];
	$postalcode = $_SESSION['billaddress_postalcode'];
	$homephone = $_SESSION['billaddress_areacode2'] . $_SESSION['billaddress_phone2'];
	$workphone = $_SESSION['billaddress_areacode'] . $_SESSION['billaddress_phone'];
}

/////////// Credentials ///////////////////
$UN = "discsox";
$PW = "m1475963m";
$Key = "d0a121f1-5f57-4a5b-b2ce-81f22fe0b5b2";

// Set up authentication
$userNamePassword = $Key . '%' . $UN . ':' . $PW ;

$contact_details = '<EmailAddress>' . $email . '</EmailAddress>
		<FirstName>' . $first_name . '</FirstName>
		<LastName>' . $last_name . '</LastName>
		<HomePhone>' . $homephone . '</HomePhone>
		<WorkPhone>' . $workphone . '</WorkPhone>
		<Addr1>' . $addr1 . '</Addr1>
		<Addr2>' . $addr2 . '</Addr2>
		<City>' . $city . '</City>
		<StateCode></StateCode>
		<StateName>' . $state . '</StateName>
		<CountryCode></CountryCode>
		<CountryName>' . $country . '</CountryName>
		<PostalCode>' . $zipcode . '</PostalCode>';
		
$contact_lists = '		
		<ContactLists>
			<ContactList id="http://api.constantcontact.com/ws/customers/' . $UN . '/lists/5" />' // This is the DiscSox NewsLine List
			//. '<ContactList id="http://api.constantcontact.com/ws/customers/' . $UN . '/lists/2" />' // Be sure to get the correct list number(s) for your list(s)
		. '</ContactLists>';

$post_entry = '<entry xmlns="http://www.w3.org/2005/Atom">
	<title type="text"> </title>
	<updated>' . date('c') . '</updated>
	<author></author>
	<id>data:,none</id>
	<summary type="text">Contact</summary>
	<content type="application/vnd.ctct+xml">
	<Contact xmlns="http://ws.constantcontact.com/ns/1.0/">
		' . $contact_details . '
		<OptInSource>ACTION_BY_CUSTOMER</OptInSource>
		' . $contact_lists . '
	</Contact>
</content>
</entry>';

//First check whether the e-mail exists in the DB
if ($debug) {echo("<p>  Contact e-mail we are looking at: " . $email. "</p> <p>&nbsp;</p>");}
//Get info by e-mail
$request ="https://api.constantcontact.com/ws/customers/" . $UN . "/contacts?email=" . $email; 

//Make the call to the server
$response = doServerCall($userNamePassword, $request);
//$response = 0;
if ($debug) {
	echo("<p>  response: " . $response. "</p> <p>&nbsp;</p>");
	echo("<p>  Newsletter Subscription: " . $subscription. "</p> <p>&nbsp;</p>");
}
	  
if (!$response) {
	echo("<p>  Your contact information has not been updated!</p> <p>&nbsp;</p>");    
} else {
	//test for errors
	//if (!(stristr($response, 'error') === FALSE)) {
	if (!stristr($response, 'error') === FALSE || strpos($response, '<') === false) {
		echo("<p>  Your contact information has not been updated!</p><p>&nbsp;</p>");
	}
	//no errors; extract the XML response data
	else {
		$data = simplexml_load_string($response);
		//echo("<P>  data: " . $data . "</P>");
	
		$contact_uri = ($data->entry->id);				
		if (!$contact_uri) {
			//contact does not exist
			if ($debug) {echo("<p> Contact does not exists! </p> <p>contact_uri: " . $contact_uri . "</P><p>&nbsp;</p>");}
			if ($subscription == "Yes") {
				//update info in database with POST	
				$request ="https://api.constantcontact.com/ws/customers/" . $UN . "/contacts" ;
				//Make the call to the server
				$postResponse = doServerCall($userNamePassword, $request,$post_entry,"POST"); 
				if ($debug) {echo("<p> Contact has been created! </p> <p>&nbsp;</p>");
				echo("<p>  postResponse: " . $postResponse. "</p>");}

			}
		}
		else {
			//contact does exist
			if ($debug) {echo("<p> Contact already exists! </p> <p> contact_uri: " . $contact_uri . "</p>");}
			if ($subscription == "Yes") {
				//update info in database with PUT
				$request = str_replace('http://', 'https://', $contact_uri);
				
				$put_entry = '<entry xmlns="http://www.w3.org/2005/Atom">
				<id>' . $contact_uri . '</id>
				<title type="text"> </title>
				<updated>' . date('c') . '</updated>
				<author></author>
				<summary type="text">Contact</summary>
				<content type="application/vnd.ctct+xml">
					<Contact xmlns="http://ws.constantcontact.com/ns/1.0/">
						' . $contact_details . '
						<OptInSource>ACTION_BY_CONTACT</OptInSource>
						' . $contact_lists . '
					</Contact>
				</content>
				</entry>';
				
				//Make the call to the server
				if ($debug) {echo("<p>  put_entry: " . $put_entry. "</p> <p>&nbsp;</p>");}
				$putResponse = doServerCall($userNamePassword, $request,$put_entry,'PUT'); 
				if ($debug) {echo("<p> Contact has been updated! </p> <p>&nbsp;</p>");
				echo("<p>  putResponse: " . $putResponse. "</p> <p>&nbsp;</p>");}
			}
			else {
				//delete contact (move to do not mail list)
				$request = str_replace('http://', 'https://', $contact_uri);
				
				//Make the call to the server
				$deleteResponse = doServerCall($userNamePassword, $request,'', 'DELETE');				
				if ($debug) {echo("<p> Contact has been removed! </p> <p>&nbsp;</p>");
				echo("<p>  deleteResponse: " . $deleteResponse. "</p> <p>&nbsp;</p>");}
				
			}
		}
	}
			

}
		
function doServerCall($userNamePWD, $request, $parameter = '', $type="GET") {
 
	// Initialize the cURL session 
	$session = curl_init($request);
		
	// Set cURL options
	curl_setopt($session, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
	curl_setopt($session, CURLOPT_USERPWD, $userNamePWD);
	curl_setopt($session, CURLOPT_HTTPHEADER, Array("Content-Type:application/atom+xml"));
	curl_setopt($session, CURLOPT_HEADER, 0); // Do not return headers
	curl_setopt($session, CURLOPT_RETURNTRANSFER, 1); // Do not display the return values in the browser window
	//curl_setopt($session, CURLOPT_RETURNTRANSFER, 0); // Display the return values in the browser window
	curl_setopt($session, CURLOPT_SSL_VERIFYPEER, 0);

	switch ($type) {
	
		case 'POST':
			//add contact
			curl_setopt($session, CURLOPT_POST, 1);
			curl_setopt($session, CURLOPT_POSTFIELDS , $parameter);
			break;
	
		case 'PUT':
			//curl_setopt($session, CURLOPT_PUT, 1);
			//curl_setopt($session, CURLOPT_PUTFIELDS, $put_entry);
			//
//			$tempfile = tmpfile();
//			fwrite($tempfile, $parameter);
//			fseek($tempfile, 0);
//			curl_setopt($session, CURLOPT_INFILE, $tempfile);
//			fclose($tempfile);
//			curl_setopt($session, CURLOPT_INFILESIZE, strlen($parameter));
//			curl_setopt($session, CURLOPT_PUT, 1);
						
			curl_setopt($session, CURLOPT_POSTFIELDS , $parameter);
			curl_setopt($session, CURLOPT_CUSTOMREQUEST, 'PUT');
			
			break;
	
		case 'DELETE':
			//delete contact
			curl_setopt($session, CURLOPT_CUSTOMREQUEST, "DELETE");	
			break;
	
		default:
			curl_setopt($session, CURLOPT_POST, 0);
			//curl_setopt ($session, CURLOPT_GET, 1);
			//curl_setopt ($session, CURLOPT_HTTPGET, 1);
			break;
	}	
	
	
	// Execute cURL session and close it
	$eResponse = curl_exec($session);
	curl_close($session);
	
	return $eResponse; # This returns HTML
		
} // end doServerCall

////////////////// END REGISTER EMAIL WITH CONSTANT CONTACT ///////////////////////
?>
</div>
</body>
</html>