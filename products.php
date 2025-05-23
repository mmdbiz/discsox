<?php
// initialize the program and read the config
include_once("include/initialize.inc");
$init = new Initialize();

// check if login is required
if($_CF['login']['require_login']){
	$login = $_Registry->LoadClass('login');
	$login->checkLogin();
}

// load the cart for the minicart display in the store template
$cart = $_Registry->LoadClass('cart');
$cart->initialize();

$vars = array();
// Load display class
$display = $_Registry->LoadClass("Display");
// run the query
$display->runQuery();
// load back vars so we can use in templates
if(count(get_object_vars($display)) > 0){
	$vars = array_merge($vars,get_object_vars($display));
}

// display results
if(count($display->records) == 0){
	
	if(count($display->subCategories) > 0){
		if(isset($_CF['product_display']['show_subcategory_thumbnails']) && !$_CF['product_display']['show_subcategory_thumbnails']){
			$_Common->printErrorMessage("No matching records were found...",
										"Please try again with different search terms.");
		}
		else{
			$template = "templates/search.result.subcategories.html";
			$productPage = $_Template->Open($template,$vars,true);
		}
	}
	else{
		$_Common->printErrorMessage("No matching records were found...",
									"Please try again with different search terms.");
	}
}
else{
	// get detail template
	if($display->detail || count($display->records) == 1){
		$template = "templates/products.display.detail.html";
	}
	// get list template
	elseif($_CF['product_display']['show_search_result_as_list']){
		$template = "templates/products.display.list.html";
	}
	// get multi-column template
	elseif($_CF['product_display']['show_search_result_using_columns']){
		$template = "templates/products.display.columns.html";
	}
	// both list and multi are false in config
	else{
		$_Common->printErrorMessage("Configuration Error:",
									"No search result template is selected in the control panel.");
	}

	$productPage = $_Template->Open($template,$vars,true);
}
// line below prints the page
?>

<?=$productPage;?>

