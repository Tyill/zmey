/* eslint-disable no-undef */
const webpack = require('webpack');

module.exports = {
  entry: [
    'webpack-dev-server/client/?http://localhost:8080',
    './tsx/app.tsx',
  ],  
  output: {
    publicPath: 'static/js/',
    path: __dirname + '/static/js/',
    filename: 'bundle.js'
  },
  devtool:  '#sourcemap',
  mode: 'development',
  resolve: {
    extensions: [".ts", ".tsx", ".js"]
  },
  module: {
    rules: [
      { test: /\.css$/, loader: 'style-loader!css-loader'},
      { test: /\.tsx?$/, exclude: /(node_modules)/, use: ['babel-loader'] },
    ]
   },
  devServer: {
    hot: true,
    // host: '192.168.1.3',
    // port: 8080,
    proxy: {
      '/api': 'http://localhost:3000',      
    },   
  },  
  plugins: [new webpack.HotModuleReplacementPlugin()]   
}