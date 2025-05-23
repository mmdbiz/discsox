<?php
class UPSOAuth {
    private $config;
    private $access_token;
    private $token_expiry;
    private $debug;
    
    public function __construct($config, $debug = false) {
        $this->config = $config;
        $this->debug = $debug;
        $this->access_token = null;
        $this->token_expiry = 0;
    }
    
    public function getAccessToken() {
        if ($this->access_token && time() < $this->token_expiry) {
            return $this->access_token;
        }
        
        return $this->refreshAccessToken();
    }
    
    private function refreshAccessToken() {
        $credentials = base64_encode($this->config['oauth_settings']['client_id'] . ':' . $this->config['oauth_settings']['client_secret']);
        
        $ch = curl_init();
        curl_setopt($ch, CURLOPT_URL, $this->config['oauth_settings']['token_endpoint']);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
        curl_setopt($ch, CURLOPT_POST, true);
        curl_setopt($ch, CURLOPT_POSTFIELDS, 'grant_type=client_credentials');
        curl_setopt($ch, CURLOPT_HTTPHEADER, [
            'Authorization: Basic ' . $credentials,
            'Content-Type: application/x-www-form-urlencoded'
        ]);
        
        $response = curl_exec($ch);
        $httpCode = curl_getinfo($ch, CURLINFO_HTTP_CODE);
        curl_close($ch);
        
        if ($httpCode !== 200) {
            throw new Exception('Failed to obtain OAuth token: ' . $response);
        }
        
        $tokenData = json_decode($response, true);
        $this->access_token = $tokenData['access_token'];
        $this->token_expiry = time() + $tokenData['expires_in'] - 60; // Buffer of 60 seconds
        
        return $this->access_token;
    }
}
?> 