<?php
// -----------------------------------------------------------------------
//         Class: Sales Tax
//       Purpose: This class does all the sales tax calculations for
//				  for the shipping and receipt pages.
// Last Modified: March 08, 2006
//
// -----------------------------------------------------------------------
class Salestax{

    public bool $debug = false;
    // these vars get passed from calcs.inc
	public string $Country = "US";
	public string $State = "";
	public string $Zip = "";
	public string $County = "";
    
    //-----------------------------------------------------
	function __construct($calcs = null){
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
	function calculateSalesTax($shipping = 0,$discount = 0){

		global $_CF;
		global $_Common;
		global $_Totals;

		$taxableTotal = $_Totals['subtotal'];
		if(isset($_Totals['cartTaxableTotal'])){
			$taxableTotal = $_Totals['cartTaxableTotal'];
		}
		$sales_tax = 0;
		
        if($this->Country == "US" && $_CF['sales_tax']['calculate_us_tax']){
            $sales_tax = $this->calculateUSSalesTax($taxableTotal,$shipping,$discount);
        }
        elseif($this->Country == "CA" && $_CF['sales_tax']['calculate_ca_tax']){
            $sales_tax = $this->calculateCanadianTax($taxableTotal,$shipping,$discount);
        }
        elseif($this->Country != "US" && $this->Country != "CA" && $_CF['sales_tax']['calculate_vat_tax']){
            $sales_tax = $this->calculateVatTax($taxableTotal,$shipping,$discount);
        }

		if($this->debug){
			$_Common->debugPrint(get_object_vars($this),"SalesTax Class Properties");	
		}

		return $_Common->format_number($sales_tax);
		
	}
	//-----------------------------------------------------
	function calculateUSSalesTax($taxableTotal,$shipping,$discount){

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
					
					$shippingPerLine = 0;
					$discountPerLine = 0;
					$count = count($_CART);
					if($shipping > 0){
						$shippingPerLine = ($shipping / $count);
					}
					if($discount > 0){
						$discountPerLine = ($discount / $count);
					}
					
					foreach($_CART as $id=>$flds){
						if(isset($flds['taxable']) && $flds['taxable']){
							if(isset($flds['tax_level']) && $flds['tax_level'] > 0){
								$rate = $rates[$flds['tax_level'] - 1];
							}
							else{
								$rate = $rates[0];
							}
							
							$subtotal = ($flds['line_total'] + $shippingPerLine);
							if($discountPerLine > 0){
								$subtotal = ($subtotal - $discountPerLine);
							}
							
							$salesTax += $_Common->format_number(floatval($subtotal) * floatval($rate));
						}
					}
				}
			}
		}
		elseif(isset($taxRates['state']) && !empty($taxRates['rate'])){
			
			// if we have a cartTaxTotal that means
			// we have item specific tax
			if($_Totals['cartTaxTotal'] != 0){
				
				// if the sequence of the calcs is not all 1,
				// we may have a discount to deduct or shipping
				// to add. We will have to break these up and
				// apply them per line so the tax comes out
				// reasonably correct.

				$shippingPerLine = 0;
				$discountPerLine = 0;
				$count = count($_CART);
				if($shipping > 0){
					$shippingPerLine = ($shipping / $count);
				}
				if($discount > 0){
					$discountPerLine = ($discount / $count);
				}
				
				foreach($_CART as $id=>$flds){
					
					if(isset($flds['is_taxable']) && $flds['is_taxable'] == "true"){
						
						if(isset($flds['tax_rate']) && $flds['tax_rate'] > 0){
							
							$lineTotal = $_Common->format_number($flds['line_total']);
							
							$taxRate = $flds['tax_rate'];
							
							// deduct the discount from the line item
							if($discountPerLine > 0){
								$lineTotal -= $discountPerLine;
							}

							// add the tax on shipping to the line item
							if($shippingPerLine > 0){
								$lineTotal += $shippingPerLine;
							}
							
							$lineTotal = $_Common->format_number($lineTotal);
							
							//$_Common->debugPrint("item specific tax line total: $lineTotal");
							
							if($lineTotal > 0){
								$salesTax += $_Common->format_number(($lineTotal * $taxRate));
							}
						}
					}
				}
			}
			else{
				if($discount > 0){
					$taxableTotal -= $discount;
				}
				$salesTax = (($taxableTotal + $shipping) * $taxRates['rate']);
			}
		}

		return $_Common->format_number($salesTax);
	}
	
	//-----------------------------------------------------
	function calculateCanadianTax($taxableTotal,$shipping,$discount){
		
		global $_CF;
		global $_Common;
		global $_DB;
		global $_Totals;
		$salesTax = 0;
		$province = strtolower($this->State);

		if($discount > 0){
			$taxableTotal -= $discount;
		}


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
	function calculateVatTax($taxableTotal,$shipping,$discount = 0){

		global $_Common;
		global $_DB;
		global $_Totals;
		$salesTax = 0;
		$country = $this->Country;

		if($discount > 0){
			$taxableTotal -= $discount;
		}

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