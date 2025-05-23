<?php
// -----------------------------------------------------------------------
//         Class: Sales Tax
//       Purpose: This class does all the sales tax calculations for
//				  for the shipping and receipt pages.
// Last Modified: March 08, 2006
//
// -----------------------------------------------------------------------
class Salestax{

    var $debug = false;
    // these vars get passed from calcs.inc
	var $Country = "US";
	var $State = "";
	var $Zip = "";
	var $County = "";
    
    //-----------------------------------------------------
	function Salestax($calcs = null){
		global $_Common;
		
		if($calcs){
			// load class vars from calcs
			foreach(get_object_vars($calcs) as $name=>$value){
				if(isset($this->$name)){
					$this->$name = $value;
				}
			}
		}
	}
	
	//-----------------------------------------------------
	function calculateSalesTax($shipping = 0){
//marcello something is wrong here...
		global $_CF;
		global $_Common;
		global $_Totals;

		$taxableTotal = $_Totals['subtotal'];
		if(isset($_Totals['cartTaxableTotal'])){
			$taxableTotal = $_Totals['cartTaxableTotal'];
		}
		$sales_tax = 0;
		
        if($this->Country == "US" && $_CF['sales_tax']['calculate_us_tax']){
            $sales_tax = $this->calculateUSSalesTax($taxableTotal,$shipping);
        }
        elseif($this->Country == "CA" && $_CF['sales_tax']['calculate_ca_tax']){
            $sales_tax = $this->calculateCanadianTax($taxableTotal,$shipping);
        }
        elseif($this->Country != "US" && $this->Country != "CA" && $_CF['sales_tax']['calculate_vat_tax']){
            $sales_tax = $this->calculateVatTax($taxableTotal,$shipping);
        }

		if($this->debug){
			$_Common->debugPrint(get_object_vars($this),"SalesTax Class Properties");	
		}

		return $_Common->format_number($sales_tax);
		
	}
	//-----------------------------------------------------
	function calculateUSSalesTax($taxableTotal,$shipping){

		global $calcs;
		global $_CART;
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Totals;

		$state = strtoupper($this->State);
		$salesTax = 0;
		$sql = "SELECT state, rate, use_tax_table, tax_table_form_field FROM sales_tax_us WHERE UPPER(state) = '$state'";
		$taxRates = $_DB->getRecord($sql);

		if($this->debug){
			$_Common->debugPrint($sql,"Tax rate SQL");
			$_Common->debugPrint($taxRates,"Tax Rates from DB");
		}

		if(isset($taxRates['use_tax_table']) && $taxRates['use_tax_table'] == "true"){

			$taxTableFile = "./$state.tax";
			$taxTableFormField = $taxRates['tax_table_form_field'];

			if(strtolower($taxTableFormField) == "shipaddress_postalcode"){
				$taxFieldValue = trim($this->Zip);
			}
			else{
				$taxFieldValue = trim($this->County);
			}

			if(file_exists($taxTableFile)){

				$taxFileRates = file($taxTableFile);
				$defaultRate = 0;
				$taxRate = 0;


				foreach($taxFileRates as $dummy=>$row){

					if(trim($row) == ""){
						continue;
					}
					$fields = array();
					$fields = explode("|",$row);
					$matchName = array_shift($fields);

					if(strtolower($matchName) == "default"){
						$taxRates['default'] = $fields;
						continue;
					}

					if(strstr($matchName,"-")){
						//found a zip range
						list($low,$high) = explode("-",$matchName);
						$low = intval($low);
						$high = intval($high);
						$taxFieldValue = intval($taxFieldValue);
						if(($taxFieldValue >= $low)&&($taxFieldValue <= $high)){
							$taxRates[$taxFieldValue] = $fields;
							break;
						}
					}
					else{
						//not a range, must be a county or a single zip
						if(strtolower($matchName) == strtolower($taxFieldValue)){
							$taxRates[$taxFieldValue] = $fields;
							break;
						}
					}
				}

				$rates = array();
				if(isset($taxRates[$taxFieldValue])){
					$rates = $taxRates[$taxFieldValue];
				}
				elseif(isset($taxRates['default'])){
					$rates = $taxRates['default'];
				}
	            
				if(count($rates) > 0){
					foreach($_CART as $id=>$flds){
						if(isset($flds['taxable']) && $flds['taxable']){
							if(isset($flds['tax_level']) && $flds['tax_level'] > 0){
								$rate = $rates[$flds['tax_level'] - 1];
							}
							else{
								$rate = $rates[0];
							}
							$subtotal = ($flds['line_total'] + $shipping);
							$salesTax += $_Common->format_number(($subtotal * $rate));
							
						}
					}
				}
			}
		}
		elseif(isset($taxRates['state']) && !empty($taxRates['rate'])){
			
			if($_Totals['cartTaxTotal'] != 0){
				// product specific tax
				$salesTax = $_Totals['cartTaxTotal'];
				// add the tax on shipping with product specific tax
				if($shipping > 0){
					$salesTax += ($shipping * $taxRates['rate']);
				}
			}
			else{
				$salesTax = (($taxableTotal + $shipping) * $taxRates['rate']);
			}
		}

		return $_Common->format_number($salesTax);
	}
	
	//-----------------------------------------------------
	function calculateCanadianTax($taxableTotal,$shipping){
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Totals;
		$salesTax = 0;
		$province = strtolower($this->State);

		$sql = "SELECT province,gst,pst,hst FROM sales_tax_ca WHERE LOWER(province) = '$province'";
		$taxRates = $_DB->getRecord($sql);
		
		if($this->debug){
			$_Common->debugPrint($taxRates,$sql);
		}
		
		if(!empty($taxRates) && strtolower($taxRates['province']) == $province){
			
			$gst = $taxRates['gst'];
			$pst = $taxRates['pst'];
			$hst = $taxRates['hst'];

			if($_CF['sales_tax']['apply_gst_and_hst_to_shipping']){
				$taxSubtotal = $taxableTotal;
				$taxSubtotal += $_Common->format_number($shipping);
				$gst = $_Common->format_number(($taxSubtotal * $gst));
				$hst = $_Common->format_number(($taxSubtotal * $hst));
				$pst = $_Common->format_number(($taxableTotal * $pst));
			}
			else{
				$gst = $_Common->format_number(($taxableTotal * $gst));
				$hst = $_Common->format_number(($taxableTotal * $hst));
				$pst = $_Common->format_number(($taxableTotal * $pst));
			}

			$_Totals['GST'] = $gst;
			$_Totals['HST'] = $hst;
			$_Totals['PST'] = $pst;

			if($this->debug){
				$_Common->debugPrint($_Totals,"CA Tax Totals");
			}

			$salesTax = $_Common->format_number(($gst + $hst + $pst));

		}
		return $salesTax;
	}
	
	//-----------------------------------------------------
	function calculateVatTax($taxableTotal,$shipping){

		global $_Common;
		global $_DB;
		global $_Totals;
		$salesTax = 0;
		$country = $this->Country;

		$sql = "SELECT country,rate FROM sales_tax_vat WHERE country = '$country'";
		$taxRates = $_DB->getRecord($sql);
		
		if($this->debug){
			$_Common->debugPrint($taxRates,$sql);
		}
		
		if(count($taxRates) > 0){
			$salesTax = $_Common->format_number((($taxableTotal + $shipping) * $taxRates['rate']));
		}
		$_Totals['VAT'] = $salesTax;
		return $salesTax;
	}
	
	
	
}
?>