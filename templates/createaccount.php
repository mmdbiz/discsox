<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
	<head>
		<title><?=$pageTitle;?></title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
		<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5">
		<link rel="stylesheet" type="text/css" href="../styles/cart.styles.css" />
</head>
<body>


<!--webbot bot="PurpleText" PREVIEW="
This page contains PHP script variables in the HTML that may be hidden in your editor.
So, please be careful editing this page and be sure to keep a backup copy before overwriting it.
View the HTML source code for more details.
"-->
	<div align="center">

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

			</script><script type="text/javascript">
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
//				alert("same as billing is checked?");
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

	  <div align="left" >
		<table border="0" cellspacing="0" cellpadding="2" width="100%" ID="Table1">
			<tr>
			  <td align="left"><h4><?=$pageTitle;?></h4></td>
		  </tr>
			<tr>
			  <td align="left">&nbsp;</td>
		  </tr>
		</table>
        <?php if(empty($_SESSION['isRegistered']) && $_CF['login']['show_registration']):?>
            <form name="order" method="get" action="shipping.php" ID="Form3">  
          <div class="newCustomer">
              <table width="100%" border="0" cellpadding="0" cellspacing="0">
                <tr bordercolor="#F0F0F0">
                  <th class="mmdHeaderRight" height="19" align="left" style="padding-left:5px;"><strong>Are you a new customer?</strong></th>
                </tr>
                <tr bordercolor="#F0F0F0">
                  <td align="left" class="tiny">&nbsp;&nbsp;</td>
                </tr>
                <tr bordercolor="#F0F0F0">
                  <td height="160" align="left" valign="top" class="fivepixPad" ><p>Please enter your email address to use as your DiscSox ID and create   a password for your account. With this ID, you can conveniently place orders, reorder,   check your recent orders and more.</p>
                    <table width="100%" border=0 cellpadding=3 cellspacing=0 id="Table6">
                      <tr>
                        <td align=right valign=middle>Login Email Address: </td>
                        <td><input type="text" name="registration[username]" value="" size=25 ID="Text25"></td>
                        </tr>
                      <?php if($_CF['login']['require_password']):?>
                      <tr>
                        <td align=right valign=middle>Choose a Password: </td>
                        <td><input type="password" name="registration[password]" value="" size=25 ID="Password2"></td>
                        </tr>
                      <?php endif;?>
                    </table>
                    <p>&nbsp;</p>
                  <p>Your account will be automatically created once you have completed the checkout.</p>              </td>
                </tr>
            </table>
        </div>
          <div class="clearboth center padding10">
            <p><a href="../privacy.htm">&nbsp;We are committed to your privacy&nbsp;</a> 
            <span class="red right"><strong>*Required Fields and Selections</strong></span></p>
          </div>
        <?php else:?>
            <form name="order" method="get" action="shipping.php" ID="Form3">
      <?php endif;?>
      
		<div class="billingInfo">
		  <table border="0" cellspacing="0" cellpadding="0" width="100%" align="left" ID="Table1">
		    <tr>
		      <td align="left">
		        <table width="100%" border="0" cellpadding="3" cellspacing="0" bordercolor="#F0F0F0" id="Table2">
		          
		          
		          <tr>
		            <th width="100%" align="left" style="padding-left:5px;"><b>Billing Information:</b></th>
	              </tr>
		          <tr>
		            <td width="100%" class="tiny">&nbsp;</td>
	              </tr>
		          <tr>
		            <td width="100%" align="center">
		              <table border="0" cellpadding="3" cellspacing="1" width="95%" id="Table3">
		                <tr>
		                  <td align="right" width="36%">First Name<strong class="red">*</strong>: </td>
		                  <td width="64%" align="left"><input type="text" size="25" name="billaddress_firstname" maxlength="25" value="<?=$_SESSION['billaddress_firstname'];?>" id="Text1">                          </td>
	                    </tr>
		                <tr>
		                  <td align="right" width="36%">Last Name<strong class="red">*</strong>: </td>
		                  <td width="64%" align="left"><input type="text" size="25" name="billaddress_lastname" maxlength="25" value="<?=$_SESSION['billaddress_lastname'];?>" id="Text2">                          </td>
	                    </tr>
		                <?php if($taxTable && $taxTableField == "shipaddress_county"):?>
		                <!-- Taxtable - County sales tax -->
		                <?php endif;?>
		                <?php if($_SESSION['isRegistered']):?>
		                <?php endif;?>
	                  </table>
		              <table width="100%" border="0" cellspacing="0" cellpadding="0"  style="display:none;">
		                <tr id="checkboxes">
		                  <td align="right" width="36%">Make Same As Billing:</td>
		                  <td width="64%" align="left"><input name="sameasbilling" type="checkbox" id="Checkbox1" onClick="shipsame(this.form);" value="1" checked>
		                    - or -
		                    <input type="checkbox" name="reset" value="1" onClick="document.forms['order'].sameasbilling.checked = false;shipsame(document.forms['order']);this.checked=false;" id="Checkbox2">
		                    Clear Fields </td>
	                    </tr>
                    </table></td>
	              </tr>
		          <tr>
		            <td height="1" class="tiny">&nbsp;&nbsp;</td>
	              </tr>
              </table></td>			
	        </tr>
	      </table>
		  </div>
        <div class="continue clearboth center"><hr size="1" noshade class="marginB10px">
	                  <!--<input name="process" class="buttons" type="submit" value="Continue"
									onclick="return checkAccount(this.form);" id="Submit1">-->
		                <input name="process" class="buttons" type="image" value="Continue" src="images/buttons/continue.png" data-inline="true" data-role="none" onclick="return continueConditionally(this.form);" id="Submit1"> 
  <!--                     <input name="process" class="buttons" type="submit" value="Continue"
									onclick="return shipsame(this.form), checkRequiredFields(this.form);" id="Submit1">-->
	                  <!--<input name="process" class="buttons" type="submit" value="Continue check fields"
                    onclick="return checkRequiredFields(this.form);" ID="Submit1">-->
		                <!--<input name="process" class="buttons" type="submit" value="Continue"
									onclick="return checkAccount(this.form), checkRequiredFields(this.form);" id="Submit1">-->
		                <?=$_CF['basics']['ship_to_billing'];?>
		                <br>
	                  Or call 1-888-347-2769 and a Customer Service representative will gladly help you.</div>
        </form>
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

	</div>
</body>
</html>