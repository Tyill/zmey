/* eslint-disable no-undef */
const webpack = require('webpack');

module.exports = {
  entry: [
    'webpack-dev-server/client/?http://localhost:8080',
    './tsx/app.tsx',
  ],  
  output: {
    publicPath: 'js/',
    path: __dirname + '/js/',
    filename: 'bundle.js'
  },
  devtool:  '#sourcemap',
  mode: 'development',
  resolve: {
    extensions: [".ts", ".tsx", '.css', ".js"],
    modules: ['node_modules']
  },
  module: {
    rules: [
      { test: /\.css$/, loader: 'style-loader!css-loader'},
      { test: /\.tsx?$/, exclude: /(node_modules)/, use: ['babel-loader'] },
      { test: /\.(woff|woff2|ttf|eot)$/, use: 'file-loader?name=fonts/[name].[ext]!static' }
    ]    
   },
  devServer: {
    hot: true,
    // host: '192.168.1.3',
    // port: 8080,
    proxy: {
      '/': 'http://localhost:5000', 
    },   
  },  
  plugins: [new webpack.HotModuleReplacementPlugin()]   
}