<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
	<head>
		<title><?=$pageTitle;?></title>
		<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
		<meta name="vs_targetSchema" content="http://schemas.microsoft.com/intellisense/ie5" />
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

		<div class="center">
			<div class="modal-body">
                <h4 id="message" class="red well well-sm col-lg-offset-4 col-lg-4 col-sm-offset-3 col-sm-6 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12"><?=$message;?></h4>
                <h4 id="resetMessage" class="red well well-sm col-lg-offset-4 col-lg-4 col-sm-offset-3 col-sm-6 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12 hide">Enter your Email below. Your password will be emailed to the matching email address in your account. </h4>
            </div>
            <div class="modal-body col-lg-offset-4 col-lg-4 col-sm-offset-3 col-sm-6 col-xs-offset-2 col-xs-8 col-xxs-offset-0 col-xxs-12">
            <div class="loginWrapper">
              <div id="regLogin">
                  <form class="form-horizontal col-sm-12" action="login.php" method="post" id="login-form" data-toggle="validator">
                    <div class="form-group has-feedback">
                      <label class="pull-left">E-Mail<span class="red">*</span></label>
                      <input class="form-control email   m-b-3" placeholder="email@you.com"  type="email" name="user" data-error="Email address is invalid" required>                      
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
                      <button type="submit" class="btn btn-danger pull-right" value="Login" >Login</button>
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
                      <input class="form-control email m-b-3" placeholder="email@you.com"  type="email" name="user_name"  value="" data-error="Email address is invalid"  id="inputEmail" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                    </div>
                    <div class="form-group has-feedback">
                      <label class="pull-left">Confirm Email<span class="red">*</span></label> 
                      <input class="form-control email" placeholder="Confirm Email" type="text"  name="user_email" value="" data-error="Email address is invalid"  id="inputEmailConfirm" data-match="#inputEmail" data-match-error="Emails don't match" required>
                      <span class="glyphicon form-control-feedback" aria-hidden="true"></span>
                      <div class="help-block with-errors"></div>
                    </div>
                    <div class="form-group">  
                      <!--<button type="submit" class="btn btn-danger pull-right" value="submit" name="forgot"  >Submit</button>-->
                      <input type="submit" class="btn btn-danger pull-right" name="forgot" value="Submit" ID="Submit2"  />
                      <div class="help-block pull-left alert alert-danger hide" id="form-error2">&nbsp; Emails don't match! Please try again. </div>
                      
                      <div class="help-block pull-left text-danger "><span class="red">*</span> Required entries</div>
                      
                    </div>
                  </form>
              </div>
            </div>
          </div>
		</div>
		<p>&nbsp;</p>
</body>
</html>