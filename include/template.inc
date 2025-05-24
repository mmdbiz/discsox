<?php
class Template{

    public bool $debug = false;
	public array $categoryList = [];
	public $siteURL = null;
	
	function __construct(){
		
	}
	// ----------------------------------------------------
	function Open($templateFile,$vars = array(),$getStoreTemplate = true,$bodyOnly = false){

		global $_CF;
		global $_CART;
		global $_CR;
		global $_Common;
		global $_DB;
		global $_isAdmin;
		global $_Registry;
		global $_Totals;
		global $website_url;
		global $secure_url;

		if(!file_exists($templateFile)){
			$templateFile = basename($templateFile);
			die("<pre><B>PROGRAM ERROR:</b> Cannot open template: $templateFile</pre>");
		}
        if(is_array($vars) && count($vars) > 0){
	        extract($vars);
	    }

		if(empty($_Totals) && !empty($_SESSION['cartTotals'])){
			$_Totals = $_SESSION['cartTotals'];	
		}
	    
		// Minicart
		$miniCart['item_count'] = empty($_Totals['totalQuantity']) ? "0" : $_Totals['totalQuantity'];
		$miniCart['total'] = empty($_Totals['subtotal']) ? "0.00" : number_format($_Totals['subtotal'],2);


		// load the template so we can get the title 
		// and content section if using store template
		
		error_reporting(E_PARSE|E_WARNING);
		ob_start();
		include_once($templateFile);
		$template = ob_get_contents();
		ob_end_clean();
		//marcello disable deprecated messages		
		//error_reporting(E_ALL);
		error_reporting(E_ALL);

		if($_isAdmin){
			$getStoreTemplate = false;
			if(strstr($templateFile,'error')){
				// replace all the tags in the html that match the {vars}
                $template = preg_replace_callback(
                    "|\{(.*?)\}|s", // Using 's' modifier as well if . needs to match newlines, original didn't specify
                    function ($matches) use ($vars) {
                        return $this->setValue($matches[1], $vars);
                    },
                    $template
                );
//				echo "got here";
//				$template = preg_replace_callback("|\{(.*?)\}|es",$this->setValue('$1',$vars),$template);
			}
		}

		// load store template
		if($getStoreTemplate && $_CF['templates']['use_store_template']){

			// get the text from the body of the page
			list($title,$contents) = $this->getPageBody($template);
			unset($template);

			if(!empty($vars['pageTitle'])){
				$title = $vars['pageTitle'];
			}
			//marcello
			//original code:
			$storeTemplate = $_CF['templates']['store_template_name'];
			//$_SESSION['DisplayDevice'] = 'mobile';
//			if (isset($_SESSION['DisplayDevice']) && $_SESSION['DisplayDevice'] == 'desktop') {
//				$storeTemplate = $_CF['templates']['store_template_name'];
//			}
//			elseif (isset($_SESSION['DisplayDevice']) && $_SESSION['DisplayDevice'] == 'mobile'){
//				$storeTemplate = "store.template_mobile.html";
//			}
//			else {
//				echo 'device not set: ' . $_SESSION['DisplayDevice'];
//			}
			
			if(!file_exists($storeTemplate)){
				$storeTemplate = basename($storeTemplate);
				die("<pre><B>PROGRAM ERROR:</b> Cannot open the store template: $storeTemplate</pre>");
				exit;
			}

			error_reporting(E_PARSE|E_WARNING);
			ob_start();
			include_once($storeTemplate);
			$template = ob_get_contents();
			ob_end_clean();
			//error_reporting(E_ALL);
			//marcello disable deprecated messages
			error_reporting(E_ALL);

			return $template;
		}
		elseif($bodyOnly){
			list($title,$contents) = $this->getPageBody($template);
			return $contents;
		}
		else{
			return $template;
		}

	}

	// -------------------------------------------------------------------
	function setValue($tag,$vars){
		if(isset($vars[$tag])){
			return trim($vars[$tag]);
		}
		else{
			return "";	
		}
	}

	// --------------------------------------------------------------
    function getPageBody($template){

        $title = $this->getPageTitle($template);

            // Pull out the body text

        $bodymatches = array();
        $match = preg_match("~<body[^>]*>(.*)</body>~",$template,$bodymatches);
        if(isset($bodymatches[1])){
            $body = $bodymatches[1];
        }
        else{
            $lines = preg_split("/\n/",$template);
            $body = "";
            $start = false;

            foreach($lines as $line){
                if(stristr($line,"<body")){
                    $start = true;
                    continue;
                }
                if(stristr($line,"</body")){
                    $start = false;
                    break;
                }
                if($start){
                    $body .= "$line\n";
                }
            }
        }

    return array($title,$body);
    }
	// --------------------------------------------------------------
	function getPageTitle($template){

        $matches = array();
        $title = "";
        if(preg_match("|<title>(.*)</title>|i",$template)){
	        $match = preg_match("|<title>(.*)</title>|i",$template,$matches);
		    $title = $matches[1];
		}

		if(trim($title) == ""){
			$title = "Weclome to our store";	
		}
		return $title;
	}

	
}
?>