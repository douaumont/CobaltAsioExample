// Copyright (c) 2024 Artyom Makarov
// This program is an example of how to use Boost.Cobalt together with Boost.Asio
// to perform read and write operations via network, as well as DNS name resolution 
// and establishing connection.
// You may use this peace of software as you wish

#include <boost/cobalt.hpp>
#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>
#include <cstdlib>
#include <iostream>
#include <iterator>
#include <string>
#include <format>
#include <string_view>
#include <algorithm>
#include <vector>
#include <fstream>

//HTTP port number
const std::string PortNum = "80";

boost::cobalt::promise<std::string> Get(const std::string& url)
{
    //similar to asio::io_context
    auto executor = co_await boost::cobalt::this_coro::executor;
    //request to send
    //note Connection header which is set to close
    const auto req = std::format("GET / HTTP/1.0\r\nHost: {}\r\nConnection: close\r\n\r\n", url);
    //boilerplate code to resolve URL
    boost::asio::ip::tcp::resolver::query query(url, PortNum, boost::asio::ip::tcp::resolver::query::numeric_service);
    boost::asio::ip::tcp:: resolver resolver(executor);
    //this is where respnose will be stored
    std::string res;
    //asio will write response here
    boost::asio::streambuf inputBuffer;
    try
    {  
        //asynchronously resolve URL
        //note using cobalt::use_op as the handler
        auto endpoints = co_await resolver.async_resolve(query, boost::cobalt::use_op);
        //use executor to initialise a socket
        boost::asio::ip::tcp::socket socket(executor);
        //asynchronously connect the socket to the resolved endpoints
        co_await boost::asio::async_connect(socket, endpoints, boost::cobalt::use_op);
        //write our request to socket
        const auto bytesWritten = co_await boost::asio::async_write(socket, boost::asio::buffer(req), boost::cobalt::use_op);
        //read the response to buffer
        //async_read will throw boost::system::system_error upon reaching end of response
        //this happens because the Connection header is set to close
        const auto bytesRead = co_await boost::asio::async_read(socket, inputBuffer, boost::cobalt::use_op);
    }
    catch(boost::system::system_error& excep)
    {
        res.resize(inputBuffer.size(), 0);
        std::istream resStream(&inputBuffer);
        //istreambuf_iterator preserves spaces and newline characters
        std::copy(std::istreambuf_iterator<char>(resStream), std::istreambuf_iterator<char>(), std::begin(res));
    }
    co_return res;
}

boost::cobalt::promise<std::vector<std::string>> PerformRequests()
{
    std::vector<std::string> urls {"example.org", "example.net"};
    std::vector<std::string> results{urls.size()};
    for (auto url = std::begin(urls); auto& res : results)
    {
        res = co_await Get(*url);
    }
    co_return results;
}

boost::cobalt::main co_main(int argc, char* argv[])
{
    auto results = co_await PerformRequests();
    for (int i = 1; const auto& res : results)
    {
        std::ofstream out(std::format("{}.html", i++));
        std::ranges::copy(res, std::ostreambuf_iterator<char>(out));
    }
    co_return EXIT_SUCCESS;
}