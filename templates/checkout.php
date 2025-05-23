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
<?php $design = true ?>
<?php if($design){ 
  //session_start();
  // initialize the program and read the config
  include_once('../include/initialize.inc');
  $init = new Initialize();
  
  // variables from the checkout.inc that we will need in this page
  $vars = array();
  
  // check if login is required
  if($_CF['login']['require_login']){
	  $login = $_Registry->LoadClass('login');
	  $login->checkLogin();
  }
  // load the cart
  $cart = $_Registry->LoadClass('cart');
  
  // get checkout page variables
  $checkout = $_Registry->LoadClass('checkout');
  $vars = get_object_vars($checkout);
  
  $label = "Billing";
  if(!$_CF['cart']['show_prices']){
	  $label = "Contact";
  }
  
  // Set the page variables
  $vars['pageTitle'] = "Checkout: $label Address Information";
  if($_CF['shipping']['require_shipping']){
	  $vars['pageTitle'] = "Checkout: $label/Shipping Address Information";
  }
};?>
<body>

<!--webbot bot="PurpleText" PREVIEW="
This page contains PHP script variables in the HTML that may be hidden in your editor.
So, please be careful editing this page and be sure to keep a backup copy before overwriting it.
View the HTML source code for more details.
"-->
	<div align="center" class="">
		<?php if(count($_CART) == 0 && !$design ): ?>
            <div>
              <h2>Your Cart is empty</h2>    
                <div class="inlineBlock m-l-3">
                  <a href="<?=$_CF['basics']['home_page_name'];?>">
                    <div class="largeSubmitButtonImg inlineBlock arrowLeft left">&nbsp;</div>
                    <div class="largeSubmitLink inlineBlock">Continue Shopping</div>
                    </a>
                </div>
            </div>
		<?php else:?>

			<?php
				error_reporting(E_PARSE|E_WARNING);
				$requiredFlds = join("','",array_keys($requiredFields));
				$showPrices = $_CF['cart']['show_prices'];
			?>

			<script type="text/javascript">
				var requiredFields = new Array('<?=$requiredFlds;?>');
				var len = requiredFields.length - 1;
				var selectedBillCountry = "<?=$_SESSION['billaddress_country'];?>";
				var selectedBillState = "<?=$_SESSION['billaddress_state'];?>";
				var selectedBillCounty = "<?=$_SESSION['billaddress_county'];?>";
				var selectedShipCountry = "<?=$_SESSION['shipaddress_country'];?>";
				var selectedShipState = "<?=$_SESSION['shipaddress_state'];?>";
				var selectedShipCounty = "<?=$_SESSION['shipaddress_county'];?>";

				<?php if($taxTable && $taxTableField == "shipaddress_county"):?>
					function countyList(){
						<?=$countyJava;?>
					}
					var counties = new countyList();
				<?php else:?>
					var counties = new Array();
				<?php endif;?>

				var hideareacode = true;
				<?php if(isset($_CF['basics']['always_display_area_code']) && $_CF['basics']['always_display_area_code']):?>
					hideareacode = false;
				<?php endif;?>

			</script>
			<script type="text/javascript">
            var selectedUserName = "registration[username]";
    
            <!--We switched the username to e-mail addresses -->
            function checkEntries(form){
                if(form.user.value == ""){
                    alert("You did not enter a valid Email Address?");
                    form.user.focus();
                    return false;
                }
                <!--So we check the login name for being an e-mail -->
                if(form.user.value != ""){
                    if(!emailCheck(form.user.value)){
                        alert("You did not enter a valid email address?");
                        form.user_email.focus();
                        return false;
                    }
                }
                <?php if($_CF['login']['require_password']):?>
                if(form.pass.value == "" && !form.forgot.checked){
                    alert("You did not enter a valid password?");
                    form.pass.focus();
                    return false;
                }
                <?php endif;?>
            return true;
            }
            function checkEntriesForgot(form){
            <!--We switched the username to e-mail addresses -->
                if(form.user_name.value == "" && form.user_email.value == ""){
                    alert("You did not enter a valid email address?");
                    form.user.focus();
                    return false;
                }
                <!--So we check the login name for being an e-mail -->
                if(form.user_name.value != ""){
                    if(!emailCheck(form.user_name.value)){
                        alert("You did not enter a valid email address?");
                        form.user_email.focus();
                        return false;
                    }
                }
                if(form.user_email.value != ""){
                    if(!emailCheck(form.user_email.value)){
                        alert("That is not a valid email address?");
                        form.user_email.focus();
                        return false;
                    }
                }
                return true;
            }
    <!-- Marcello _______________________________________ -->
            function checkShipTo(form,field){
                    var fldIndex = form.elements[field].selectedIndex;
                    var stid = form.elements[field].options[fldIndex].value;
                    if (form.elements[field].options[fldIndex].text == 'My Billing Address') {
    //					alert("same as billing is checked?");
    //				    Address is same as billing
                        showHideShip(false);
                        form.sameasbilling.checked =  true;
                        <?php $_CF['basics']['ship_to_billing'] = true; ?>
                        shipsame(form);
                    }
                    else{				
    //					alert("different address is checked?");
                        showHideShip(true);		
                        form.sameasbilling.checked =  false;	
                        <?php $_CF['basics']['ship_to_billing'] = false; ?>
                        shipsame(form);
                    }			
            }
            function checkShipTo2(form,field){
                    var fldIndex = form.elements[field].selectedIndex;
                    var stid = form.elements[field].options[fldIndex].value;
                    if (form.elements[field].options[fldIndex].text == 'My Billing Address') {
    //					alert("same as billing is checked?");
    //				    Address is same as billing
                        document.forms['hideaddrform'].submit();
                    }
                    else{				
    //					alert("different address is checked?");
                        document.forms['showaddrform'].submit();
                    }			
            }
            function showHideShip(whichOne){
    //			This sets the visibility of the ship to table	
                if(whichOne){
                    showIt('Table4',true);
                    showIt('billToReq',true);
                    showIt('shipToReq',false);		
                }
                else{
                    showIt('Table4',false);
                    showIt('billToReq',false);
                    showIt('shipToReq',true);		
                }
            }
            function continueConditionally(form){
    //			var fieldsChecked = checkRequiredFields(form);
    
                if (form.sameasbilling.checked) {
    				//alert("same as billing is checked?");
                    shipsame(form);
                }
    //			if(fieldsChecked){
                if(checkRequiredFields(form)){
    //				alert("fields have been checked?");
                    return true;
                }
                return false;
            }
    //	
    <!-- Marcello _______________________________________ -->
            function showForgot(doforgot){
                if(doforgot){
                    showIt('message',false);
                    showIt('Table9',false);
                    showIt('Table10',true);
                }
                else{
                    showIt('message',true);
                    showIt('Table9',true);
                    showIt('Table10',false);
                }
            }
            function showIt(whichEl,show){
                if(document.all){
                    whichEl = document.all[whichEl];
                }
                else{
                    whichEl = document.getElementById(whichEl);
                }
                if(whichEl){
                    if(show){
                        whichEl.style.display = "";
                    }
                    else{
                        whichEl.style.display = "none";
                    }
                }
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
			<script type="text/javascript" src="javascripts/checkout.js"></script>


<!-- DON'T REMOVE THIS FORM. IT'S USED WHEN SOMEONE MAKES
				A SELECTION ON THIS PAGE THAT REQUIRES US TO RELOAD THE PAGE. -->
			<?php if(count($addressBook) > 1):?>
				<form name="addrform" action="checkout_show_billaddr.php" method="get" ID="Form2">
					<input type="hidden" name="csid" ID="Hidden2">
				</form>
			<?php endif;?>
            
				<form name="hideaddrform" action="checkout.php" method="get" ID="Form5">
				</form>
				<form name="showaddrform" action="checkout_show_billaddr.php" method="get" ID="Form6">
				</form>

			<!-- <form name="order" method="get" action="shipping.php" ID="Form3">-->

	  <div align="left" class="row m-x-0-xxs">
          <div class="col-sm-offset-1 col-sm-10">
            <!-- check if logged in -->
			<?php if(!empty($_SESSION['isRegistered']) && $_CF['login']['show_registration']):?>
            
                <button type="button" class="btn btn-danger pull-left m-t-1 hide" value="Login" id="custShipFieldsBackTop"><span class="glyphicon glyphicon-triangle-left" aria-hidden="true"></span> Back</button>
                
                <button type="button" class="btn btn-danger pull-right m-t-1 hide" value="Login" name="process" id="custShipFieldsContinueTop">Continue <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
                <button type="button" class="btn btn-danger pull-right m-t-1 " value="Login" onclick="return continueConditionally(Form3);" id="billingAddrContinueTop">Continue <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
            
            <?php else:?>
                <button type="button" class="btn btn-danger pull-left m-t-1 hide" value="Login" id="custShipFieldsBackTop"><span class="glyphicon glyphicon-triangle-left" aria-hidden="true"></span> Back</button>
                
                <button type="button" class="btn btn-danger pull-right m-t-1 hide" value="Login" name="process" id="custShipFieldsContinueTop">Continue <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
                <button type="button" class="btn btn-danger pull-left m-t-1 hide" value="Login" id="billingAddrBackTop"><span class="glyphicon glyphicon-triangle-left" aria-hidden="true"></span> Back</button>
                <button type="button" class="btn btn-danger pull-right m-t-1 hide" value="Login" onclick="return continueConditionally(Form3);" id="billingAddrContinueTop">Continue <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
			<?php endif;?>
            
          <h2 class="text-center">Checkout <span id="billingTitle" class="hide">  -> Billing</span><span id="shippingTitle" class="hide">  -> Paymethod & Shipping</span></h2>
          </div>
                  
        <div id="custInfoContainer">
          <!-- check if logged in -->
          <?php if(empty($_SESSION['isRegistered']) && $_CF['login']['show_registration']):?>            
            <!-- Not logged in --> 
            
            <!--Existing Customers-->
            <div id="existingCustomers" class="col-sm-offset-1 col-sm-4 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12">
              <div class="existingCustMsg">
                <h4 id="message" class="red well well-sm">Returning Customers</h4>
                <h4 id="resetMessage" class="red well well-sm hide">Enter your Email below. Your password will be emailed to the matching email address in your account. </h4>
              </div>
              <div class="existingCustForm loginWrapper">
                <div id="regLogin">
                  <form class="form-horizontal col-sm-12" action="login.php" method="post" id="login-form" data-toggle="validator">
                    <div class="form-group has-feedback">
                      <label class="pull-left">E-Mail<span class="red">*</span></label>
                      <input class="form-control email" placeholder="email@you.com" type="email" name="user" data-error="Email address is invalid" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                    </div>
                    <div class="form-group has-feedback">
                      <label class="pull-left">Password<span class="red">*</span></label> <span class="pull-right" id="forgotPWD"><a href="#">Forgot your password?</a></span>
                      <input class="form-control" placeholder="*****" type="password" name="pass" data-minlength="4" data-error="Minimum of 4 characters" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                    </div>
                    <div class="form-group">  
                      <button type="submit" class="btn btn-danger pull-right" value="Login" >Login <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
                      <div class="help-block pull-left alert alert-danger hide" id="form-error">&nbsp; The form is not valid! Please try again. </div>
                      
                      <div class="help-block pull-left text-danger "><span class="red">*</span> Required entries</div>
                      
                    </div>
                  </form>
                </div>
                  
                <span class="pull-right hide" id="backToLogin"><a href="#">Back to Login</a></span>
                
                <div id="resetPWD" class="">
                  <form class="form-horizontal col-sm-12" action="login.php" method="post" id="reset-form" data-toggle="validator">
                    <div class="form-group has-feedback">
                      <label class="pull-left">Email<span class="red">*</span></label>
                      <input class="form-control email m-b-3" placeholder="email@you.com" type="email" name="user_name"  value="" data-error="Email address is invalid"  id="inputEmail" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                    </div>
                    <div class="form-group has-feedback">
                      <label class="pull-left">Confirm Email<span class="red">*</span></label> 
                      <input class="form-control email" placeholder="Confirm Email" type="email"  name="user_email" value="" data-error="Email address is invalid"  id="inputEmailConfirm" data-match="#inputEmail" data-match-error="Emails don't match" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                    </div>
                    <div class="form-group">  
                      <button type="submit" class="btn btn-danger pull-right" value="submit" name="forgot"  >Submit <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
                      <!--<input type="submit" name="forgot" value="Submit" ID="Submit2"  />-->
                      <div class="help-block pull-left alert alert-danger hide" id="form-error2">&nbsp; Emails don't match! Please try again. </div>
                      
                      <div class="help-block pull-left text-danger "><span class="red">*</span> Required entries</div>
                      
                    </div>
                  </form>
                </div>
              </div>
              </div>
            <!--divider left-->
            <div id="centerDividerLeft" class="centerDivider col-sm-offset-0 col-sm-1 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12"></div>
            
            <form class="form-horizontal" name="order" method="get" action="shipping.php" ID="Form3" data-toggle="validator"> 
            <!--New Customers-->
            <div id="newCustomers" class="col-sm-offset-1 col-sm-4 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12">
              <div class="newCustMsg">
                <h4 id="" class="red well well-sm">New Customers</h4>
                
              </div>
              <div class="newCustForm">
                <div id="newCustLogin">
                  <div class="form-group has-feedback m-x-0">
                    <label class="pull-left">Login E-Mail<span class="text-primary">*</span></label>
                    <input class="form-control email" placeholder="login-email@you.com (optional)" type="email" name="registration[username]" value="" data-error="Email address is invalid" data-placement="top" data-toggle="tooltip" data-original-title="Leave blank for Guest Checkout!">                  
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div> 
                  <div class="form-group has-feedback m-x-0">
                    <label class="pull-left">Login Password<span class="text-primary">*</span></label> 
                    <input class="form-control" placeholder="***** (optional)" type="password"  name="registration[password]" value="" data-minlength="4" data-error="Minimum of 4 characters" data-placement="top" data-toggle="tooltip" data-original-title="Leave blank for Guest Checkout!" >
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
                  <div class="form-group m-x-0" id="newCustButtonContainer">  
                    <button type="button" id="newCustbutton" data-placement="top" data-toggle="tooltip" data-original-title="No Account will be created when leaving Login Email and Login Password blank!" class="btn btn-danger pull-right" value="Login" >Continue <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
                    <div class="help-block pull-left alert alert-danger hide" id="form-error">&nbsp; The form is not valid! Please try again. </div>
                    
                    <div class="help-block pull-left text-danger "><span class="text-primary">*Leave blank for Guest Checkout</span></div>
                    
                  </div>
                  
                  <div id="newCustDetailsLeft" class="form-inline row moveAway">
                    <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                      <label class="pull-left">Area-Code<span class="red">*</span></label> 
                      <input class="form-control" placeholder="+1 888" type="text" name="billaddress_areacode" value="<?=$_SESSION['billaddress_areacode'];?>" data-error="Area code is required" maxlength="8" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                    </div>
                    <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                      <label class="pull-left">Phone<span class="red">*</span></label> 
                      <input class="form-control" placeholder="347-2769" type="text" name="billaddress_phone" value="<?=$_SESSION['billaddress_phone'];?>" data-error="Phone # is required" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                    </div>
                    
                    <div class="form-group has-feedback m-x-0 col-md-12">
                      <label class="pull-left">E-Mail<span class="red">*</span></label>
                      <input class="form-control email" placeholder="notification-email@you.com" type="email" name="billaddress_email" value="<?=$_SESSION['billaddress_email'];?>" data-error="Email address is invalid" data-placement="top" data-toggle="tooltip" data-original-title="Required for sending confirmation email!" required>                  
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                    </div> 
                  </div>
                </div>
              </div>
              </div>
            <!--divider right-->            
            <div id="centerDividerRight" class=" hide centerDivider col-sm-offset-0 col-sm-1 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12"></div>
          <?php else:?>
          <!-- when logged in -->          
          <form class="form-horizontal" name="order" method="get" action="shipping.php" id="Form3" data-toggle="validator">
            <?php endif;?>
            <!-- End logged in Check -->
            
            
            <div id="newCustDetailsRight" class="col-sm-5 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12  <?php if((empty($_SESSION['isRegistered']) && $_CF['login']['show_registration'])):?>col-sm-offset-1 moveAway<?php else:?>col-sm-offset-3<?php endif;?>">        
              
              <!-- Billing Info -->          
              <div class="billingInfo">
                <div class="billingInfoMsg">
                  <h4 id="" class="red well well-sm">Billing Address</h4>
                </div>
                
                <div class="form-group has-feedback m-x-0 m-b-0">
                  <label class="pull-left">Company</label>
                  <input class="form-control" placeholder="Company name" type="text" name="billaddress_companyname" value="<?=$_SESSION['billaddress_companyname'];?>" >
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                <div class="form-inline row">
                  <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                    <label class="pull-left">First Name<span class="red">*</span></label>
                    <input class="form-control" placeholder="First name" type="text" name="billaddress_firstname" value="<?=$_SESSION['billaddress_firstname'];?>" data-error="First name is required" required>
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
                  <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                    <label class="pull-left">Last Name<span class="red">*</span></label>
                    <input class="form-control" placeholder="Last name" type="text" name="billaddress_lastname" value="<?=$_SESSION['billaddress_lastname'];?>" data-error="Last name is required" required>                  
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
                  <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                    <label class="pull-left">Address<span class="red">*</span></label>
                    <input class="form-control"  data-placement="top" data-toggle="tooltip" data-original-title="We cannot ship to a P.O. Box! " placeholder="Address" type="text" name="billaddress_addr1" value="<?=$_SESSION['billaddress_addr1'];?>" data-error="Address is required" required>                  
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
                  <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                    <label class="pull-left">Apt, Suite...</label>
                    <input class="form-control" placeholder="Apt, Suite etc. (optional)" type="text" name="billaddress_addr2" value="<?=$_SESSION['billaddress_addr2'];?>">
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>    
                  </div>
                  <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                    <label class="pull-left">Country<span class="red">*</span></label>
                    <select class="form-control" name="billaddress_country" onChange="checkCountry(this.form,this.name);" id="countrySelect" required>
                      <option value="US" selected>United States </option>
                      <option value="AR">Argentina </option>
                      <option value="AU">Australia </option>
                      <option value="AT">Austria </option>
                      <option value="BE">Belgium </option>
                      <option value="BM">Bermuda </option>
                      <option value="BR">Brazil </option>
                      <option value="BG">Bulgaria </option>
                      <option value="CA">Canada </option>
                      <option value="CN">China, Peoples Republic of </option>
                      <option value="CR">Costa Rica </option>
    			  	  <option value="HR">Croatia </option>
                      <option value="CZ">Czech Republic </option>
                      <option value="DK">Denmark </option>
                      <option value="DO">Dominican Republic </option>
                      <option value="EN">England </option>
                      <option value="EE">Estonia </option>
                      <option value="FI">Finland </option>
                      <option value="FR">France </option>
                      <option value="DE">Germany </option>
                      <option value="GR">Greece </option>
                      <option value="HO">Holland </option>
                      <option value="HK">Hong Kong </option>
                      <option value="HU">Hungary </option>
                      <option value="IS">Iceland </option>
                      <option value="IN">India </option>
                      <option value="ID">Indonesia </option>
                      <option value="IE">Ireland </option>
                      <option value="IL">Israel </option>
                      <option value="IT">Italy </option>
                      <option value="JM">Jamaica </option>
                      <option value="JP">Japan </option>
                      <option value="LI">Liechtenstein </option>
                      <option value="LU">Luxembourg </option>
                      <option value="MY">Malaysia </option>
                      <option value="MT">Malta </option>
                      <option value="MX">Mexico </option>
                      <option value="MC">Monaco </option>
                      <option value="NL">Netherlands </option>
                      <option value="NZ">New Zealand </option>
                      <option value="NB">Northern Ireland </option>
                      <option value="NO">Norway </option>
                      <option value="PH">Philippines </option>
                      <option value="PL">Poland </option>
                      <option value="PT">Portugal </option>
                      <!--<option value="PR">Puerto Rico </option>-->
                      <option value="IE">Republic of Ireland </option>
                      <option value="RO">Romania </option>
                      <option value="RU">Russia </option>
                      <option value="SF">Scotland </option>
                      <option value="RS">Serbia </option>
                      <option value="SG">Singapore </option>
                      <option value="ZA">South Africa </option>
                      <option value="ES">Spain </option>
                      <option value="SE">Sweden </option>
                      <option value="CH">Switzerland </option>
                      <option value="TW">Taiwan </option>
                      <option value="TH">Thailand </option>
                      <option value="TR">Turkey </option>
                      <option value="AE">United Arab Emirates </option>
                      <option value="GB">United Kingdom </option>
                      <option value="VU">Vanuatu </option>
                      <option value="WL">Wales </option>
                    </select>
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
                  <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                    <label class="pull-left">City<span class="red">*</span></label>
                    <input class="form-control" placeholder="City (APO, FPO or DPO)" type="text" name="billaddress_city" value="<?=$_SESSION['billaddress_city'];?>" data-error="City is required" required>
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
                  <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6 stateSelect">
                    <label class="pull-left">State/Province<span class="red">*</span></label>
                    <select  class="form-control" name="billaddress_state" onChange="addCounties(this.form,this.name);" id="stateSelect" data-error="State is required" data-stateprov="INVALID" required>
                      <option value="INVALID">Select State</option>
                    </select>
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
                  <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                    <label class="pull-left">Postal Code<span class="red">*</span></label>
                    <input class="form-control" placeholder="Postal Code/Zip" type="text" name="billaddress_postalcode" value="<?=$_SESSION['billaddress_postalcode'];?>" data-error="Postal Code is required" required>                  
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
                  
                </div>
                
                <!-- check if logged in -->
                <?php if(!empty($_SESSION['isRegistered']) && $_CF['login']['show_registration']):?>
                <!--New Cust Details left - show only when logged in-->
                  <div id="newCustDetailsLeft" class="form-inline row ">
                    <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                      <label class="pull-left">Area-Code<span class="red">*</span></label> 
                      <input class="form-control" placeholder="+1 888" type="text" name="billaddress_areacode" value="<?=$_SESSION['billaddress_areacode'];?>" data-error="Area code is required" maxlength="8" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                      </div>
                    <div class="form-group has-feedback m-x-0  col-md-6 col-sm-12">
                      <label class="pull-left">Phone<span class="red">*</span></label> 
                      <input class="form-control" placeholder="347-2769" type="text" name="billaddress_phone" value="<?=$_SESSION['billaddress_phone'];?>" data-error="Phone # is required" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                      </div>
                    
                    <div class="form-group has-feedback m-x-0 col-sm-12 col-md-12">
                      <label class="pull-left">E-Mail<span class="red">*</span></label>
                      <input class="form-control email" placeholder="notification-email@you.com" type="email" name="billaddress_email" value="<?=$_SESSION['billaddress_email'];?>" data-error="Email address is invalid" data-placement="top" data-toggle="tooltip" data-original-title="Required for sending confirmation email!" required>                  
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                      </div> 
                  </div>
                  <!-- Billing Address Continue Button ONLY -->
                  <div class="form-group"> 
                    <button type="button" class="btn btn-danger pull-right m-r-2" value="Login" onclick="return continueConditionally(this.form);" id="billingAddrContinue">Continue <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
                  </div>
                <?php else:?>
                  <!-- Billing Address Continue and Back Button -->
                 <div class="form-group"> 
                    <button type="button" class="btn btn-danger pull-right m-r-2" value="Login" onclick="return continueConditionally(this.form);" id="billingAddrContinue">Continue <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>                   
                    <button type="button" class="btn btn-danger pull-left m-l-2" value="Login" id="billingAddrBack"><span class="glyphicon glyphicon-triangle-left" aria-hidden="true"></span> Back</button>
                 </div>
                <?php endif;?>
                <!-- End logged in Check -->
                
              </div>
              <!--End Billing Info --> 
              
              <!--<div class="continue clearboth center"><hr size="1" noshade class="marginB10px">
             <div class="form-group" >
              <input name="process" class="buttons" type="image" value="Continue" src="images/buttons/continue.png" data-inline="true" data-role="none" onclick="return continueConditionally(this.form);" id="Submit1"> 
              </div>
              <?=$_CF['basics']['ship_to_billing'];?>
              <br>
              Or call 1-888-347-2769 and a Customer Service representative will gladly help you.
            </div>-->
            </div>
            
            <div id="NewCustPayMethod"  class="col-sm-offset-1 col-sm-4 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12 moveAway " >
              <div class="newCustPayMethMsg">
                <h4 id="" class="red well well-sm">Payment Method</h4>
              </div>
              <!--Payment Methods -->
              <?php if(count($paymentMethods) > 1):?>
              <table border="0" cellpadding="3" cellspacing="1" width="100%" id="Table5">
                <tr>
                  <td width="15" align="left"> </td>
                  <td align="left">
                    <?php foreach($paymentMethods as $pageName=>$method):?>
                    <?php 
                        if($method == 'Paypal') {
                          $method = "PayPal/Credit Card";
                        }
                      ?>
                    <?php if($pageName == $_CF['payment_methods']['default_payment_method']):?>
                    <input name="payment_method" type="radio" value="<?=$pageName;?>" checked data-role="none">
                    <img class="absmiddle" src="images/<?=$pageName.".png";?>" > <?php echo str_replace("Payment", "Check", $method); ?>
                    <?php else:?>
                    <input name="payment_method" type="radio" value="<?=$pageName;?>"  data-role="none">
                    <img class="absmiddle" src="images/<?=$pageName.".png";?>" > <?php echo str_replace("Payment", "Check", $method); ?>
                    <?php endif;?>
                    <br>
                  <?php endforeach;?></td>
                </tr>
                </table>
              <?php elseif(count($paymentMethods) == 1):?>
              <?php if($paymentPage != ""):?>
              <?=$paymentPage;?>
              <?php else:?>
              <input type="hidden" name="payment_method" value="<?=$_CF['payment_methods']['default_payment_method'];?>" id="Hidden3" data-role="none">
              <?php endif;?>
              <?php endif;?>
              
              <!--Coupons-->
              <?php if($haveCoupons):?>
              <div class="CustCouponMsg m-t-4">
                <h4 id="" class="red well well-sm m-b-2">Promotions</h4>
              </div>
              <div class="form-group has-feedback m-x-0">
                <label class="pull-left">Coupon/Promo Code:</label>
                <input class="form-control" placeholder="Coupon" type="text" name="coupon" >
                <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                <div class="help-block with-errors"></div>
                </div>
              <!--<table border="0" cellpadding="3" cellspacing="1" width="100%" id="Table8">
		            <tr>
		              <td align="right" width="50%">Coupon/Promo Code:</td>
		              <td align="left"><input type="text" name="coupon" size="20" id="Text22"></td>
	                </tr>
	              </table>-->
              <?php endif;?>
              <!--End Payment Methods -->
              
              <!-- Shipping Preferences--> 
              <div class="shipingInfo">
              <div class="shipPrefs m-t-4">
                <h4 id="" class="red well well-sm m-b-2">Shipping Preferences</h4>
              </div>
              <div class="form-group m-x-0 m-b-0 has-feedback">
                <label class="pull-left">Address Type<span class="red">*</span></label>
                <select class="form-control" name="shipaddress_delivery_type" id="shipaddress_delivery_type">
				  <?php if(!empty($_SESSION['shipaddress_delivery_type']) && $_SESSION['shipaddress_delivery_type'] == "commercial"):?>
                      <option value="residential">Residential</option>
                      <option value="commercial" selected>Commercial</option>
                  <?php else:?>
                      <option value="residential" selected>Residential</option>
                      <option value="commercial">Commercial</option>
                  <?php endif;?>
                </select>
                <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                <div class="help-block with-errors"></div>
              </div>              
              
			  <?php if(count($shippingPlugins) > 1):?>              
                  <div class="form-group m-x-0 m-b-0 has-feedback">
                    <label class="pull-left">Preferred Ship Method<span class="red">*</span></label>
                    <select class="form-control" name="preferred_shipper">
					  <?php foreach($shippingPlugins as $k=>$shipper):?>
                      <?php if($_SESSION['preferred_shipper'] == $shipper):?>
                      <option value="<?=$shipper;?>" selected>
                      <?=strtoupper($shipper);?>
                      </option>
                      <?php else:?>
                      <option value="<?=$shipper;?>">
                      <?=strtoupper($shipper);?>
                      </option>
                      <?php endif;?>
                      <?php endforeach;?>
                    </select>
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
                  
			  <?php elseif(count($shippingPlugins) == 1):?>
                  <div class="form-group m-x-0 has-feedback">
                    <label class="pull-left">Shipping Insurance</label>
                    <input type="checkbox" name="insurance2" value="true">
                    <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                    <div class="help-block with-errors"></div>
                  </div>
			  <?php endif;?>
                        
                        &nbsp;Use USPS  for small domestic, international&nbsp; <br>
                          &nbsp;                      &amp; APO orders!&nbsp;
              
              
              </div>
            </div>
            
            <!-- ship info-->
            <div id="custShipFields"  class="col-sm-offset-1 col-sm-5 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12 moveAway " >
              <div class="newCustPayMethMsg">
                <h4 id="" class="red well well-sm">Shipping Address</h4>
              </div>
              <div class="addressSettings form-horizontal">
                <?php if($_SESSION['isRegistered']):?>
                  <div class="form-group">
                    <label class="control-label col-xs-5">Update this Address:     </label>
                    <div class="col-xs-3">
                        <label class="radio-inline">
                            <input type="radio" name="billaddress_update" value="true" id="Radio1"> Yes
                        </label>
                    </div>
                    <div class="col-xs-4">
                        <label class="radio-inline">
                            <input type="radio" name="billaddress_update" value="false" checked="true" id="Radio2"> No
                        </label>
                    </div>        
                  </div>
                <?php endif;?>
                <div class="form-group">
                  <label class="control-label col-xs-5">Same As Billing:</label>
                  <div class="col-xs-3">
                      <label class="checkbox-inline">
                          <input name="sameasbilling" type="checkbox" id="Checkbox1" onClick="shipsame(this.form);" value="1" checked="true"> <div class="m-l-2"> - or - </div>
                      </label>
                  </div>
                  <div class="col-xs-4">
                      <label class="checkbox-inline">
                          <input type="checkbox" name="reset" value="1" onClick="document.forms['order'].sameasbilling.checked = false;shipsame(document.forms['order']);this.checked=false;" id="Checkbox2">
                    Clear Fields 
                      </label>
                  </div>        
                </div>
              </div>
              
              <!--Ship to Fields-->
              <div class="form-group has-feedback m-x-0 m-b-0">
                <label class="pull-left">Company</label>
                <input class="form-control" placeholder="Company name" type="text" name="shipaddress_companyname" value="<?=$_SESSION['shipaddress_companyname'];?>" >
                <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                <div class="help-block with-errors"></div>
                </div>
              
              <div class="form-inline row">
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                  <label class="pull-left">First Name<span class="red">*</span></label>
                  <input class="form-control" placeholder="First name" type="text" name="shipaddress_firstname" value="<?=$_SESSION['shipaddress_firstname'];?>" data-error="First name is required" required>
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                  <label class="pull-left">Last Name<span class="red">*</span></label>
                  <input class="form-control" placeholder="Last name" type="text" name="shipaddress_lastname" value="<?=$_SESSION['shipaddress_lastname'];?>" data-error="Last name is required" required>                  
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                  <label class="pull-left">Address<span class="red">*</span></label>
                  <input class="form-control"  data-placement="top" data-toggle="tooltip" data-original-title="We cannot ship to a P.O. Box! " placeholder="Address line 1" type="text" name="shipaddress_addr1" value="<?=$_SESSION['shipaddress_addr1'];?>" data-error="Address is required" required>                  
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                  <label class="pull-left">Apt, Suite...</label>
                  <input class="form-control" placeholder="Apt, Suite etc. (optional)" type="text" name="shipaddress_addr2" value="<?=$_SESSION['shipaddress_addr2'];?>">
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>    
                </div>
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                  <label class="pull-left">Country<span class="red">*</span></label>
                  <select class="form-control" name="shipaddress_country" onChange="checkCountry(this.form,this.name);" id="countrySelect" required>
                    <option value="US" selected>United States </option>
                    <option value="AR">Argentina </option>
                    <option value="AU">Australia </option>
                    <option value="AT">Austria </option>
                    <option value="BE">Belgium </option>
                    <option value="BM">Bermuda </option>
                    <option value="BR">Brazil </option>
                    <option value="BG">Bulgaria </option>
                    <option value="CA">Canada </option>
                    <option value="CN">China, Peoples Republic of </option>
                    <option value="CR">Costa Rica </option>
    			  	<option value="HR">Croatia </option>
                    <option value="CZ">Czech Republic </option>
                    <option value="DK">Denmark </option>
                    <option value="DO">Dominican Republic </option>
                    <option value="EN">England </option>
                    <option value="EE">Estonia </option>
                    <option value="FI">Finland </option>
                    <option value="FR">France </option>
                    <option value="DE">Germany </option>
                    <option value="GR">Greece </option>
                    <option value="HO">Holland </option>
                    <option value="HK">Hong Kong </option>
                    <option value="HU">Hungary </option>
                    <option value="IS">Iceland </option>
                    <option value="IN">India </option>
                    <option value="ID">Indonesia </option>
                    <option value="IE">Ireland </option>
                    <option value="IL">Israel </option>
                    <option value="IT">Italy </option>
                    <option value="JM">Jamaica </option>
                    <option value="JP">Japan </option>
                    <option value="LI">Liechtenstein </option>
                    <option value="LU">Luxembourg </option>
                    <option value="MY">Malaysia </option>
                    <option value="MT">Malta </option>
                    <option value="MX">Mexico </option>
                    <option value="MC">Monaco </option>
                    <option value="NL">Netherlands </option>
                    <option value="NZ">New Zealand </option>
                    <option value="NB">Northern Ireland </option>
                    <option value="NO">Norway </option>
                    <option value="PH">Philippines </option>
                    <option value="PL">Poland </option>
                    <option value="PT">Portugal </option>
                    <!--<option value="PR">Puerto Rico </option>-->
                    <option value="IE">Republic of Ireland </option>
                    <option value="RO">Romania </option>
                    <option value="RU">Russia </option>
                    <option value="SF">Scotland </option>
                    <option value="RS">Serbia </option>
                    <option value="SG">Singapore </option>
                    <option value="ZA">South Africa </option>
                    <option value="ES">Spain </option>
                    <option value="SE">Sweden </option>
                    <option value="CH">Switzerland </option>
                    <option value="TW">Taiwan </option>
                    <option value="TH">Thailand </option>
                    <option value="TR">Turkey </option>
                    <option value="AE">United Arab Emirates </option>
                    <option value="GB">United Kingdom </option>
                    <option value="VU">Vanuatu </option>
                    <option value="WL">Wales </option>
                  </select>
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                  <label class="pull-left">City<span class="red">*</span></label>
                  <input class="form-control" placeholder="City (APO, FPO or DPO)" type="text" name="shipaddress_city" value="<?=$_SESSION['shipaddress_city'];?>" data-error="City is required" required>
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6 stateSelect">
                  <label class="pull-left">State/Province<span class="red">*</span></label>
                  <select  class="form-control" name="shipaddress_state" onChange="addCounties(this.form,this.name);" id="stateSelect" data-error="State is required" data-stateprov="INVALID" required>
                    <option value="INVALID">Select State</option>
                  </select>
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                  <label class="pull-left">Postal Code<span class="red">*</span></label>
                  <input class="form-control" placeholder="Postal Code/Zip" type="text" name="shipaddress_postalcode" value="<?=$_SESSION['shipaddress_postalcode'];?>" data-error="Postal Code is required" required>                  
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                  <label class="pull-left">Area-Code<span class="red">*</span></label> 
                  <input class="form-control" placeholder="+1 888" type="text" name="shipaddress_areacode" value="<?=$_SESSION['shipaddress_areacode'];?>" data-error="Area code is required" maxlength="8" required>
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                <div class="form-group has-feedback m-x-0 col-sm-12 col-md-6">
                  <label class="pull-left">Phone<span class="red">*</span></label> 
                  <input class="form-control" placeholder="347-2769" type="text" name="shipaddress_phone" value="<?=$_SESSION['shipaddress_phone'];?>" data-error="Phone # is required" required>
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                
                <div class="form-group has-feedback m-x-0 col-md-12">
                  <label class="pull-left">E-Mail<span class="red">*</span></label>
                  <input class="form-control email" placeholder="notification-email@you.com" type="email" name="shipaddress_email" value="<?=$_SESSION['shipaddress_email'];?>" data-error="Email address is invalid" data-placement="top" data-toggle="tooltip" data-original-title="Required for sending confirmation email!" required>                  
                  <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                  <div class="help-block with-errors"></div>
                </div>
                
                <div class="addressBook form-horizontal">
                  <?php if($_SESSION['isRegistered']):?>
                    <div class="form-group">
                    <label class="control-label col-xs-5">Address Book:</label>
                    <div class="col-xs-7">
                        <label class="checkbox-inline">
                            <input type="checkbox" name="shipaddress_savenew" value="true" id="Checkbox3">
   Save Address as new 
                        </label>
                    </div>
                    <div class="col-xs-7 col-xs-offset-5">
                        <label class="checkbox-inline">
                            <input type="checkbox" name="shipaddress_update" value="true" id="Checkbox4">
                      Update Address
                        </label>
                    </div>        
                  </div>
                  <?php endif;?>
                  
                </div>
                <button type="button" class="btn btn-danger pull-left m-l-2" value="Login" id="custShipFieldsBack"><span class="glyphicon glyphicon-triangle-left" aria-hidden="true"></span> Back</button>
                <button type="submit" class="btn btn-danger pull-right m-r-2" value="Login" name="process">Continue <span class="glyphicon glyphicon-triangle-right" aria-hidden="true"></span></button>
                </div>
              
            </div>
          </form>
        </div>
	  </div>


			<script type="text/javascript">
//				var selectedShipPreference = "<?=$_CF['basics']['web_site_url'];?>";
//				var selectedShipPreference = "<?=$_CF['basics']['ship_to_billing'];?>";
				selectBoxes(document.forms['order']);
//				checkShipTo(document.forms['order'],'selectShipToDisplay');
//    			if(selectedShipPreference){
//					alert("same as billing is checked?");
//					document.forms['order'].sameasbilling.checked = true;
//				}
//				else{
//					alert("same as billing is NOT checked?");
//					document.forms['order'].sameasbilling.checked = false;
//				}
//				alert("selectedShipPreference value is: "+ selectedShipPreference);
			</script>			

		<?php endif;?>
	</div>
</body>
</html>