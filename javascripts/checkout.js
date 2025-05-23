
/*
This javascript is used in the checkout and payment forms to
select, test, and show/hide fields based on the users selections.
*/

// -------------------------------------------------------------------
// FUNCTIONS USED IN SHIPPING PAGE AND PAYMENT FORMS
// -------------------------------------------------------------------
function selectCountry(form){

	if(shipaddress_country != "" && form.shipaddress_country){
		selectLen = form.shipaddress_country.length;
		for(i=0;i<=(selectLen -1); i++){
			if(form.shipaddress_country.options[i].value == shipaddress_country){
				form.shipaddress_country.options[i].selected = true;
				break;
			}
		}
	}
	// Load country, state/province and shipping options
	checkCountry(form,'shipaddress_state');

	// Now select state
	if(shipaddress_state != "" && form.shipaddress_state){
		selectLen = form.shipaddress_state.length;
		for(i=0;i<=(selectLen -1); i++){
			if(form.shipaddress_state.options[i].value == shipaddress_state){
				form.shipaddress_state.options[i].selected = true;
				break;
			}
		}
	}
}

// -------------------------------------------------------------------
function checkCountry(form,fldname){

var CountryIndex;
var CountryValue;
var stateField;
var postalCodeField;
var postalCodeId;
var countyField;
var countyId;

	if(fldname == "shipaddress_country" && form.shipaddress_country){
		CountryIndex = form.shipaddress_country.selectedIndex;
		CountryValue = form.shipaddress_country.options[CountryIndex].value;
		stateField = "shipaddress_state";
		postalCodeField = "shipaddress_postalcode";
		postalCodeId = "shippostalcode";
		countyField = "shipaddress_county";
		countyId = "shipcounties";
	}

	if(fldname == "billaddress_country" && form.billaddress_country){
		CountryIndex = form.billaddress_country.selectedIndex;
		CountryValue = form.billaddress_country.options[CountryIndex].value;
		stateField = "billaddress_state";
		postalCodeField = "billaddress_postalcode";
		postalCodeId = "billpostalcode";
		countyField = "billaddress_county";
		countyId = "billcounties";
	}

	var PostalCodeCountries = new Array('US','CA','JP');

	if(CountryValue == "US"){
		clearStates(form,stateField);
		addStates(form,stateField);
		showField(form,'vat',false);
		showField(form,'billaddress_areacode',true);
		showField(form,'shipaddress_areacode',true);
		
		if(typeof(counties) != 'undefined' && typeof(form.elements[countyField]) != 'undefined') {
			addCounties(form,fldname);
		}
	}
	else{
		if(CountryValue == "CA"){
			clearStates(form,stateField);
			addProvinces(form,stateField);
			showField(form,'vat',false);
			showField(form,'billaddress_areacode',true);
			showField(form,'shipaddress_areacode',true);
		}
		else{
			if(eval(hideareacode)){
				if(fldname == 'billaddress_country'){
					showField(form,'billaddress_areacode',false);
					if(typeof(form.elements['billaddress_phone']) != 'undefined'){
						form.elements['billaddress_phone'].size = 15;
						form.elements['billaddress_phone'].maxlength = 15;
					}
				}
				if(fldname == 'shipaddress_country'){
					showField(form,'shipaddress_areacode',false);
					if(typeof(form.elements['shipaddress_phone']) != 'undefined'){
						form.elements['shipaddress_phone'].size = 15;
						form.elements['shipaddress_phone'].maxlength = 15;
					}
				}
			}

            if(typeof(form.elements[stateField]) != 'undefined'){
    			clearStates(form,stateField);
	   	       	var noState = new Option("NONE","NONE",false,false);
	       		form.elements[stateField].options[0] = noState;
            }

			if(typeof(form.elements[postalCodeField]) != 'undefined'){
				//form.elements[postalCodeField].value = "";
			}
			showField(form,'vat',true);
		}

		clearCounties(form,countyField);
		showField(form,countyId,false);


	}
	if(eval(form.elements[postalCodeField])){
		for(i=0;i<=PostalCodeCountries.length -1;i++){
			if(PostalCodeCountries[i] == CountryValue){
				showField(form,postalCodeId,true);
				break;
			}
			else{
				showField(form,postalCodeId,false);
			}
		}
	}

	if(eval(form.shipping_method)){
		loadShippingMethods(form);
	}
}

// -------------------------------------------------------------------
function checkState(form){

var taxField = "shipaddress_state";
var orderFormStateField = "shipaddress_state";

var CountryIndex = form.shipaddress_country.selectedIndex;
var CountryValue = form.shipaddress_country.options[CountryIndex].value;
var CountryText = form.shipaddress_country.options[CountryIndex].text;
var taxFieldValue = "";
var taxFieldType = form.elements[taxField].type;
var fieldValues = new Array();

	if(taxFieldType == "select-one"){
		var taxIndex = form.elements[taxField].selectedIndex;
		fieldValues = form.elements[taxField].options[taxIndex].value.split('|');
		taxFieldValue = fieldValues[0];
	}
	else{
		fieldValues = form.elements[taxField].value.split('|');
		taxFieldValue = fieldValues[0];
	}

	if((CountryValue == "US")&&((taxFieldValue == "")||(taxFieldValue == "INVALID"))){
		alert("\nSince you selected " + CountryText + " for your country, you" +
			"\nmust select your ship to state to continue.\n");
		form.elements[taxField].focus();
	return false;
	}

	if((CountryValue == "CA")&&((taxFieldValue == "")||(taxFieldValue == "INVALID"))){
		alert("\nSince you selected " + CountryText + " for your country, you" +
			"\nmust select your ship to province to continue.\n");
		form.elements[taxField].focus();
	return false;
	}

	if(CountryValue != "US" && CountryValue != "CA"){
		if(eval(form.elements['vat_number'])){
			if(form.elements['vat_number'].value != ""){
				var vatNum = form.elements['vat_number'].value;
				if(!validateVatNumFmt(CountryValue,vatNum)){
					return false;
				}
			}
		}
	}

return true;
}

// -------------------------------------------------------------------
function clearStates(form,fld){
    if(typeof(form.elements[fld]) != 'undefined'){
    	selectLen = form.elements[fld].length;
    	for ( i = (selectLen -1); i>=0; i--){
    		form.elements[fld].options[i] = null;
    	}
    }
}

// -------------------------------------------------------------------
function clearCounties(form,fld){
	if(typeof(form.elements[fld]) != 'undefined' && form.elements[fld].type == 'select-one'){
		selectLen = form.elements[fld].length;
		for ( i = (selectLen -1); i>=0; i--){
			form.elements[fld].options[i] = null;
		}
		var noCounty = new Option("Out of state","out_of_state",false,false);
		form.elements[fld].options[0] = noCounty;
	}
}
// -------------------------------------------------------------------

function addCounties(form,fld){

	var stateIndex;
	var stateValue;
	var CountyField;
	var CountyId;

	if(fld == "billaddress_state"){
		stateIndex = form.billaddress_state.selectedIndex;
		stateValue = form.billaddress_state.options[stateIndex].value;
		CountyField = "billaddress_county";
		CountyId = "billcounties";
	}

	if(fld == "shipaddress_state"){
		stateIndex = form.shipaddress_state.selectedIndex;
		stateValue = form.shipaddress_state.options[stateIndex].value;
		CountyField = "shipaddress_county";
		CountyId = "shipcounties";
	}


	if(typeof(counties) != 'undefined' && eval(counties[stateValue]) && typeof(form.elements[fld]) != 'undefined'){
		clearCounties(form,CountyField);
		if(counties[stateValue].length > 0){
			for(i=0;i<=counties[stateValue].length -1;i++){
				if(counties[stateValue][i] != ""){
					var str = counties[stateValue][i];
					var newCounty = new Option(str,str,false,false);
					form.elements[CountyField].options[i] = newCounty;
				}
			}
			showField(form,CountyId,true);
		}
	}
	else{
		if(typeof(form.elements[CountyField]) != 'undefined'){
			clearCounties(form,CountyField);
			showField(form,CountyId,false);
		}
	}
}

// ------------------------------------------------------------------

function showField(form,whichEl,show){

	if(whichEl == "vat" && form.shipaddress_country){
		var VATCountries = new Array('AT','BE','DK','FI','FR','DE','EL','IE','IT','LU','NL','PT','ES','SE','GB');
		var CountryIndex = form.shipaddress_country.selectedIndex;
		var CountryValue = form.shipaddress_country.options[CountryIndex].value;
		for(i=0;i<=VATCountries.length -1;i++){
			if(VATCountries[i] == CountryValue){
				show = true;
				break;
			}
			else{
				show = false;
			}
		}
	}
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


// -------------------------------------------------------------------
function addProvinces(form,fld){

var Provinces = new Array();

// Entries are "ABBREVIATION|PROVINCE";

Provinces[0]  = "Alberta|Alberta";
Provinces[1]  = "British Columbia|British Columbia";
Provinces[2]  = "Labrador|Labrador";
Provinces[3]  = "Manitoba|Manitoba";
Provinces[4]  = "Native Reservations|Native Reservations";
Provinces[5]  = "Nunavut|Nunavut";
Provinces[6]  = "New Brunswick|New Brunswick";
Provinces[7]  = "Newfoundland|Newfoundland";
Provinces[8]  = "North West Territories|North West Territories";
Provinces[9]  = "Nova Scotia|Nova Scotia";
Provinces[10] = "Ontario|Ontario";
Provinces[11] = "Prince Edward Island|Prince Edward Island";
Provinces[12] = "Quebec|Quebec";
Provinces[13] = "Saskatchewan|Saskatchewan";
Provinces[14] = "Yukon|Yukon";

	for(i=0;i<=Provinces.length -1;i++){
		if(Provinces[i] != null){
		var pro = Provinces[i].split('|');
		var newProvince = new Option(pro[1],pro[0],false,false);
		form.elements[fld].options[i] = newProvince;
		}
	}
}

// -------------------------------------------------------------------
function addStates(form,fld){

var States = new Array();

// Entries are "ABBREVIATION|STATE";

States[0]  = "INVALID|- Select State -";
States[1]  = "AL|Alabama";
States[2]  = "AK|Alaska";
States[3]  = "AZ|Arizona";
States[4]  = "AR|Arkansas";
States[5]  = "CA|California";
States[6]  = "CO|Colorado";
States[7]  = "CT|Connecticut";
States[8]  = "DE|Delaware";
States[9]  = "DC|District of Columbia";
States[10] = "FL|Florida";
States[11] = "GA|Georgia";
States[12] = "GU|Guam";
States[13] = "HI|Hawaii";
States[14] = "ID|Idaho";
States[15] = "IL|Illinois";
States[16] = "IN|Indiana";
States[17] = "IA|Iowa";
States[18] = "KS|Kansas";
States[19] = "KY|Kentucky";
States[20] = "LA|Louisiana";
States[21] = "ME|Maine";
States[22] = "MD|Maryland";
States[23] = "MA|Massachusetts";
States[24] = "MI|Michigan";
States[25] = "MN|Minnesota";
States[26] = "MS|Mississippi";
States[27] = "MO|Missouri";
States[28] = "MT|Montana";
States[29] = "NE|Nebraska";
States[30] = "NV|Nevada";
States[31] = "NH|New Hampshire";
States[32] = "NJ|New Jersey";
States[33] = "NM|New Mexico";
States[34] = "NY|New York";
States[35] = "NC|North Carolina";
States[36] = "ND|North Dakota";
States[37] = "OH|Ohio";
States[38] = "OK|Oklahoma";
States[39] = "OR|Oregon";
States[40] = "PA|Pennsylvania";
States[41] = "PR|Puerto Rico";
States[42] = "RI|Rhode Island";
States[43] = "SC|South Carolina";
States[44] = "SD|South Dakota";
States[45] = "TN|Tennessee";
States[46] = "TX|Texas";
States[47] = "UT|Utah";
States[48] = "VT|Vermont";
States[49] = "VA|Virginia";
States[50] = "VI|Virgin Islands";
States[51] = "WA|Washington";
States[52] = "WV|West Virginia";
States[53] = "WI|Wisconsin";
States[54] = "WY|Wyoming";
States[55] = "AA|Armed Forces (AA)";
States[56] = "AE|Armed Forces (AE)";
States[57] = "AP|Armed Forces (AP)";

	for(i=0;i<=States.length -1;i++){
		if(States[i] != null){
		var values = States[i].split('|');
		var newState = new Option(values[1],values[0],false,false);
		form.elements[fld].options[i] = newState;
		}
	}
}

// -------------------------------------------------------------------
function clearMethods(form){
	selectLen = form.shipping_method.length;
	for ( i = (selectLen -1); i>=0; i--){
		form.shipping_method.options[i] = null;
	}
}

// -------------------------------------------------------------------
function loadShippingMethods(form){

	// This loads the values for the shipping rates based
	// on the country selected.

clearMethods(form);
var CountryIndex = form.shipaddress_country.selectedIndex;
var CountryValue = form.shipaddress_country.options[CountryIndex].value;

	if(!eval(methods[CountryValue])){
		CountryValue = "INT";
	}
	if(eval(methods[CountryValue])){
		for(i=0;i<=methods[CountryValue].length -1;i++){
			if(methods[CountryValue][i] != ""){
				var values = methods[CountryValue][i].split(':');
				var newMethod = new Option(values[0] + " - " + values[1],values[0],false,false);
				form.shipping_method.options[i] = newMethod;
			}
		}
	}

	form.shipping_method.options[0].selected = true;
}

//-------------------------------------------------------------------
function validateVatNumFmt(country,vatNum){

vatNum = vatNum.toLowerCase();

VATRATES = new Array();
VATRATES['AT'] = "9,[u]\\d{8}";
VATRATES['BE'] = "9,\\d{9}";
VATRATES['DK'] = "8,\\d{8}";
VATRATES['FI'] = "8,\\d{8}";
VATRATES['FR'] = "11,\\d{11}:[a-hj-np-z]\\d{10}:\\d[a-hj-np-z]\\d{9}:[a-hj-np-z][a-hj-np-z]\\d{9}";
VATRATES['DE'] = "9,\\d{9}";
VATRATES['EL'] = "8,\\d{8}";
VATRATES['IE'] = "8,\\d{7}[a-z]:\\d[a-z]\\d{5}[a-z]";
VATRATES['IT'] = "11,\\d{11}";
VATRATES['LU'] = "8,\\d{8}";
VATRATES['NL'] = "12,\\d{9}[b]\\d\\d";
VATRATES['PT'] = "9,\\d{9}";
VATRATES['ES'] = "9,[a-z]\\d{9}:\\d{9}[a-z]:[a-z]\\d{8}[a-z]";
VATRATES['SE'] = "12,\\d{10}01";
VATRATES['GB'] = "9,\\d{9}";

if(VATRATES[country] == null){
	return true;
}

var len = VATRATES[country].split(',')[0];
var pattern = VATRATES[country].split(',')[1];

	if(vatNum.length > len){
		var tooLong = (vatNum.length - len);
		alert("Your VAT Number is " + tooLong + " characters too long?");
	return false;
	}
	if(vatNum.length < len){
		var tooShort = (len - vatNum.length);
		alert("Your VAT Number is " + tooShort + " characters too short?");
	return false;
	}

	var match = false;

	if(pattern.match(/\:/g)){
		var patterns = new Array();
		patterns = pattern.split(':');
		for(k=0;k<patterns.length;k++){
			if(vatNum.match(patterns[k])){
					alert(vatNum.match(patterns[k]));
					match = true;
					break;
			}
		}
	}
	else{
		if(vatNum.match(pattern)){
			match = true;
		}
	}
	if(match){
		return true;
	}
	else{
		alert("Your VAT number " + vatNum + " is invalid for " + country);
		return false;
	}

return false;
}


// -------------------------------------------------------------------
// FUNCTIONS USED ONLY ON THE PAYMENT FORMS
// -------------------------------------------------------------------
function testRequiredFields(form){

	// Loop through the form fields and
	// test each of the required fields

	if(typeof(requiredFields) != 'undefined'){

		for(j = 0; j < requiredFields.length; j++){

			var bCountry = null;

            if(requiredFields[j].substring(0,4) == "ship" && typeof(form.shipaddress_country) != 'undefined'){
                var sCountryIdx = form.shipaddress_country.selectedIndex;
                var bCountry = form.shipaddress_country.options[sCountryIdx].value;
            }
            else if(typeof(form.billaddress_country) != 'undefined'){
                var bCountryIdx = form.billaddress_country.selectedIndex;
                var bCountry = form.billaddress_country.options[bCountryIdx].value;
            }

			if((bCountry != "CA" && bCountry != "US") && (requiredFields[j] == 'billaddress_areacode' || requiredFields[j] == 'shipaddress_areacode')){
				continue;
			}

			if(typeof(form.elements[requiredFields[j]]) != 'undefined'){

				var label = "";
				for (i=0; i < requiredFields[j].length; i++) {
					character = requiredFields[j].charAt(i);
					if ("_".indexOf(character) != -1){
						label += " ";
					}
					else{
						label += character;
					}
				}

				var requiredValue = "";

				if(form.elements[requiredFields[j]].type == "text"){
					requiredValue = form.elements[requiredFields[j]].value;
				}
				if(form.elements[requiredFields[j]].type == "select-one"){
					var sIndex = form.elements[requiredFields[j]].selectedIndex;
					requiredValue = form.elements[requiredFields[j]].options[sIndex].value;
				}
				if(form.elements[requiredFields[j]].type == "radio"){
					for(i=0; i<form.elements[requiredFields[j]].length; i++) {
						if(form.elements[requiredFields[j]][i].checked) {
							requiredValue = form.elements[requiredFields[j]][i].value;
							break;
						}
					}
				}
				if(form.elements[requiredFields[j]].type == "checkbox"){
					if(form.elements[requiredFields[j]].checked){
						requiredValue = form.elements[requiredFields[j]].value;
					}
				}

				requiredValue = requiredValue.replace(/^\s*|\s*$/g,"");

				if(requiredValue == "" || requiredValue.toLowerCase() == "invalid" || requiredValue.toLowerCase() == "default"){
					if(form.elements[requiredFields[j]].type == "text"){
						alert("You forgot to fill in the \"" + label + "\" field.\n" +
								"This field is required before processing your order.");
					}
					else{
						alert("You forgot to make a selection for the \"" + label + "\" field.\n" +
								"This field is required before processing your order.");
					}

					form.elements[requiredFields[j]].focus();
					return false;
				}

				var testField = requiredFields[j].substring(12);

				if(requiredFields[j] == "card_number"){
					if(requiredValue != ""){
						var cleanFld = strip(requiredValue);
						form.elements[requiredFields[j]].value = cleanFld;
					}
				}
				if(testField == "email"){
					if(!emailCheck(requiredValue)){
                        alert("Please enter a valid email address");
                        form.elements[requiredFields[j]].select();
                        return false;
					}
				}
				if((bCountry == "US" || bCountry == "CA") && testField == "areacode"){
                    var areacode = strip(requiredValue);
					if(areacode.length < 3 || areacode.length > 3 || parseInt(areacode) == 0){
                        alert("Please enter a valid " + label);
                        form.elements[requiredFields[j]].select();
                        return false;
					}
				}
				if((bCountry == "US" || bCountry == "CA") && testField == "phone"){
                    var phone = strip(requiredValue);
					if(phone.length < 7 || phone.length > 7 || parseInt(phone) == 0){
                        alert("Please enter a valid " + label);
                        form.elements[requiredFields[j]].select();
                        return false;
					}
				}
				if((bCountry == "CA" || bCountry == "US") && testField == "postalcode"){

                    if(!isZip(requiredValue) && bCountry == "US"){
                        alert("Please enter a valid " + label);
                        form.elements[requiredFields[j]].select();
                        return false;
					}

                    // remove spaces
                    requiredValue = requiredValue.replace(' ', '');

                    if(bCountry == "CA" && (requiredValue.length < 6 || requiredValue.length > 6)){
                        alert("Please enter a valid " + label);
                        form.elements[requiredFields[j]].select();
                        return false;
					}
				}
			}
		}
	}
	convertApostrophies(form);

return true;
}

// -------------------------------------------------------------------
function convertApostrophies(form){
	for(j = 0; j < form.elements.length; j++){
		if(form.elements[j].type == "text"){
			var strText = form.elements[j].value;
			form.elements[j].value = strText.replace("'","`");
		}
	}
}



// -------------------------------------------------------------------
function checkRequiredFields(form){

	// Checks to make sure the various
	// form fields are filled in

	// First check the required fields - not needed anymore Marcello

//	var Required = testRequiredFields(form);
//	if(!Required){
//		return false;
//	}

	if(typeof(form.sameasbilling) != 'undefined' && !form.sameasbilling.checked && form.shipaddress_addr1.value == ""){
		form.sameasbilling.checked = true;
		shipsame(form);
	}


	if(typeof(form.card_number) != 'undefined'){

		// Then the payment fields
		// Comment (//) these next to lines for testing:

		var Payment = checkCard(form);
		if(!Payment){
			return false;
		}

		var Expire = checkExpireDate(form);
		if(!Expire){
			return false;
		}
	}
	else{
		return true;
	}
}

// -------------------------------------------------------------------
function checkCard(form){

	// Checks the credit card field for
	// empty and invalid card entries

	var cardNumField = "card_number";
	var cardField = form.elements[cardNumField];
	var entry = form.elements[cardNumField].value;

	if(entry == ""){
		alert('You did not enter a valid credit card number\n' +
			'Please check your entry and try again.');
		cardField.focus();
		return false;
	}
	var strippedEntry = strip(entry);
	if((!isCreditCard(strippedEntry))||(strippedEntry.length == 0)){
		alert('The credit card number you entered could not be validated.\n' +
				'Please check the number and try again.');
		cardField.focus();
		cardField.select();
		return false;
	}

	if(!checkCardType(form)){
		return false;
	}


return true;
}


// -------------------------------------------------------------------
function checkCardType(form) {

	var typeField = "credit_card_type";
	var typeIndex = form.elements[typeField].selectedIndex;
	var typeFieldValue = form.elements[typeField].options[typeIndex].value;

	var numField = "card_number";
	var numFieldValue = form.elements[numField].value;
	var num = numFieldValue.substring(0,1);

	var blnOK = true;
	if((typeFieldValue.toLowerCase() == "american express")&&(num != 3)){
		blnOK = false;
	}
	if((typeFieldValue.toLowerCase() == "american_express")&&(num != 3)){
		blnOK = false;
	}
	if((typeFieldValue.toLowerCase() == "amex")&&(num != 3)){
		blnOK = false;
	}
	if((typeFieldValue.toLowerCase() == "visa")&&(num != 4)){
		blnOK = false;
	}
	if((typeFieldValue.toLowerCase() == "mastercard")&&(num != 5)){
		blnOK = false;
	}
	if((typeFieldValue.toLowerCase() == "discover")&&(num != 6)){
		blnOK = false;
	}

	if(!blnOK){
		alert("The credit card number that you entered does not match the card type that you selected from the available card types that we offer.\n\n" +
			"Please select the correct card type for the credit card.");
		form.elements[typeField].focus();
		return false;
	}

return true;
}



// -------------------------------------------------------------------
function checkExpireDate(form) {

	// make sure the date is in the future

	var monthIndex = form.elements['expire_month'].selectedIndex;
	var expireMonth = form.elements['expire_month'].options[monthIndex].value - 1;

	var yearIndex = form.elements['expire_year'].selectedIndex;
	var expireYear = form.elements['expire_year'].options[yearIndex].value;

	var now = new Date();
    var today = new Date(now.getYear(),now.getMonth(),now.getDate());

	var monthDays = new Array(31,28,31,30,31,30,31,31,30,31,30,31);
	var expireDay = monthDays[expireMonth];
	var expire = new Date(expireYear,expireMonth,expireDay);

	if(expire < today){
		alert("The expiration date you selected is invalid.\n\n" +
				"Please select a valid expiration date for the credit card.");
		form.elements['expire_month'].focus();
		return false;
	}

return true;
}

// -------------------------------------------------------------------
function isCreditCard(st) {

	// Tests the credit card number.
	// Encoding only works on cards
	// with less than 19 digits

	if (st.length > 19)
		return (false);
	sum = 0; mul = 1; l = st.length;
	for (i = 0; i < l; i++) {
		digit = st.substring(l-i-1,l-i);
		tproduct = parseInt(digit ,10)*mul;
		if (tproduct >= 10)
			sum += (tproduct % 10) + 1;
		else
			sum += tproduct;
		if (mul == 1)
			mul++;
		else
			mul--;
	}
	if ((sum % 10) == 0)
		return (true);
	else
		return (false);
}
// -------------------------------------------------------------------
function strip(val) {

	// Strips the dashes, spaces, etc
	// from the credit card number

val = "" + val;
	if (!val)
		return "";
	var result = "";
	for (i=0; i < val.length; i++) {
		character = val.charAt(i);
		if ("0123456789".indexOf(character) != -1)
		result += character;
	}
return result;
}


// -------------------------------------------------------------------
function shipsame(form){
	
	var stateIndex;//marcello
	
	if(form.sameasbilling.checked){
		//alert ("got to shipsame");

		form.shipaddress_companyname.value = form.billaddress_companyname.value;
		form.shipaddress_firstname.value = form.billaddress_firstname.value
		form.shipaddress_lastname.value = form.billaddress_lastname.value;
		form.shipaddress_addr1.value = form.billaddress_addr1.value;
		form.shipaddress_addr2.value = form.billaddress_addr2.value;
		form.shipaddress_city.value = form.billaddress_city.value;
		form.shipaddress_postalcode.value = form.billaddress_postalcode.value;
		form.shipaddress_areacode.value = form.billaddress_areacode.value;
		form.shipaddress_phone.value = form.billaddress_phone.value;
		form.shipaddress_email.value = form.billaddress_email.value;

		// Marcello Ship state needs to be set
		stateIndex = form.billaddress_state.selectedIndex;	
		form.shipaddress_state.selectedIndex = form.billaddress_state.selectedIndex;
		form.shipaddress_state.options[stateIndex].value = form.billaddress_state.options[stateIndex].value;

		// Marcello If Ship state has county tax then display set county fields
		addCounties(form,"shipaddress_state")

		if(form.billaddress_country.type == "select-one"){
			var bCountryIdx = form.billaddress_country.selectedIndex;
			form.shipaddress_country.options[bCountryIdx].selected = true;
		}
		else{
			form.shipaddress_country.value = form.billaddress_country.value;
		}


		if(typeof(form.shipaddress_county) != 'undefined' && typeof(form.billaddress_county) != 'undefined'){
			if(form.shipaddress_county.type == 'select-one'){
				var bCountyIdx = form.billaddress_county.selectedIndex;
				form.shipaddress_county.options[bCountyIdx].selected = true;
			}
			else{
				form.shipaddress_county.value = form.billaddress_county.value;
			}
		}

			// reload the states/provinces

		checkCountry(form,'shipaddress_country');

		if(form.billaddress_state.type == "select-one"){
			var bStateIdx = form.billaddress_state.selectedIndex;
			form.shipaddress_state.options[bStateIdx].selected = true;
		}
		else{
			form.shipaddress_state.value = form.billaddress_state.value;
		}

	}
	else{
		form.shipaddress_companyname.value = "";
		form.shipaddress_firstname.value = "";
		form.shipaddress_lastname.value = "";
		form.shipaddress_addr1.value = "";
		form.shipaddress_addr2.value = "";
		form.shipaddress_city.value = "";
		form.shipaddress_postalcode.value = "";
		form.shipaddress_areacode.value = "";
		form.shipaddress_phone.value = "";
		form.shipaddress_email.value = "";

			// reload the previously selected country/state/province

		selectBoxes(form,true);
	}
}

//----------------------------------------------------------------------------
function selectBoxes(form,shipOnly){

	if(!shipOnly){
		for(j=0;j<=form.elements['billaddress_country'].length -1;j++){
			if(form.elements['billaddress_country'].options[j].value == selectedBillCountry){
				form.elements['billaddress_country'].options[j].selected = true;
				break;
			}
		}
		checkCountry(form,'billaddress_country');
	}

	if(form.elements['shipaddress_country']){
		for(j=0;j<=form.elements['shipaddress_country'].length -1;j++){
			if(form.elements['shipaddress_country'].options[j].value == selectedShipCountry){
				form.elements['shipaddress_country'].options[j].selected = true;
				break;
			}
		}
		checkCountry(form,'shipaddress_country');
	}

	if(selectedBillState == "INVALID" && (selectedShipState != "INVALID" || selectedShipState != "")){
		selectedBillState = selectedShipState;
	}

		//load states/provinces

	if(!shipOnly){
        if(typeof(form.elements['billaddress_state']) != 'undefined'){
    		for(j=0;j<=form.elements['billaddress_state'].length -1;j++){
    			if(form.elements['billaddress_state'].options[j].value == selectedBillState){
    				form.elements['billaddress_state'].options[j].selected = true;
    				addCounties(form,'billaddress_state');
    				break;
    			}
    		}
        }
	}
	if(typeof(form.elements['shipaddress_state']) != 'undefined'){
		for(j=0;j<=form.elements['shipaddress_state'].length -1;j++){
			if(form.elements['shipaddress_state'].options[j].value == selectedShipState){
				form.elements['shipaddress_state'].options[j].selected = true;
				addCounties(form,'shipaddress_state');
				break;
			}
		}
	}

}

//---------------------------------------------------------------
function selectExpireDate(form){
	var date = new Date();
	var month = date.getMonth();
	var yy = date.getYear();
	var year = (yy < 1000) ? yy + 1900 : yy;
	if(form.elements['expire_month']){
		form.elements['expire_month'].options[month].selected = true;
	}
	if(form.elements['expire_year']){
		for(j=0;j<=form.elements['expire_year'].length -1;j++){
			if(form.elements['expire_year'].options[j].value == year){
				form.elements['expire_year'].options[j].selected = true;
				break;
			}
		}
	}
}
//---------------------------------------------------------------
function submitAddressChange(form,field){
	if(document.forms['addrform'].elements['csid']){
		var fldIndex = form.elements[field].selectedIndex;
		var csid = form.elements[field].options[fldIndex].value;
		document.forms['addrform'].elements['csid'].value = csid;
		document.forms['addrform'].submit();
	}
}

//---------------------------------------------------------------
function submitShipMethodChange(form,field){

	if(document.forms['shipform'].elements['shipping_method']){
		
		var fldIndex = form.elements[field].selectedIndex;
		var selectedMethod = form.elements[field].options[fldIndex].value;

		//alert("\nthis is the ship method form This is the shipping value we pass: " + selectedMethod + "\n \n");
		document.forms['shipform'].elements['shipping_method'].value = selectedMethod;
		document.forms['shipform'].submit();
	}
}

//Marcello add carrier change funcion
//---------------------------------------------------------------
function submitShipCarrierChange(form,field){


	if(document.forms['shipform2'].elements['preferred_shipper']){

		var fldIndex = form.elements[field].selectedIndex;
		var selectedMethod = form.elements[field].options[fldIndex].value;

		document.forms['shipform2'].elements['preferred_shipper'].value = selectedMethod;
		document.forms['shipform2'].submit();		
		//alert("\nthis is the preferred shipper form This is the value we pass: " + selectedMethod + "\n \n");
	}
}

//---------------------------------------------------------------
function submitPaymentChange(form,field){
	if(document.forms['payform'].elements['payment_method']){
		var fldIndex = form.elements[field].selectedIndex;
		var csid = form.elements[field].options[fldIndex].value;
		document.forms['payform'].elements['payment_method'].value = csid;
		document.forms['payform'].submit();
	}
}

//---------------------------------------------------------------
function disable(form,el){
	//form.elements['process'].disabled = true;
}

// -------------------------------------------------------------------
function isZip(s){
    // Check for correct zip code
    reZip = new RegExp(/(^\d{5}$)|(^\d{5}-\d{4}$)/);
    if (!reZip.test(s)) {
         return false;
    }
return true;
}

// -------------------------------------------------------------------
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

